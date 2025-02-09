/*-------------------------------------------------------------------------------
 This file is part of distributional random forest (drf).
 
 drf is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 drf is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with drf. If not, see <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------*/

#include <algorithm>
#include <memory>

#include "commons/DefaultData.h"
#include "tree/TreeTrainer.h"

namespace drf {

TreeTrainer::TreeTrainer(std::unique_ptr<RelabelingStrategy> relabeling_strategy,
                         std::unique_ptr<SplittingRuleFactory> splitting_rule_factory,
                         std::unique_ptr<OptimizedPredictionStrategy> prediction_strategy) :
    relabeling_strategy(std::move(relabeling_strategy)),
    splitting_rule_factory(std::move(splitting_rule_factory)),
    prediction_strategy(std::move(prediction_strategy)) {}

std::unique_ptr<Tree> TreeTrainer::train(const Data& data,
                                         RandomSampler& sampler,
                                         const std::vector<size_t>& clusters,
                                         const TreeOptions& options) const {
  std::vector<std::vector<size_t>> child_nodes;
  std::vector<std::vector<size_t>> nodes;
  std::vector<size_t> split_vars;
  std::vector<double> split_values;

  child_nodes.emplace_back();
  child_nodes.emplace_back();
  create_empty_node(child_nodes, nodes, split_vars, split_values);

  std::vector<size_t> new_leaf_samples;

  if (options.get_honesty()) {
    std::vector<size_t> tree_growing_clusters;
    std::vector<size_t> new_leaf_clusters;
    sampler.subsample(clusters, options.get_honesty_fraction(), tree_growing_clusters, new_leaf_clusters);

    sampler.sample_from_clusters(tree_growing_clusters, nodes[0]);
    sampler.sample_from_clusters(new_leaf_clusters, new_leaf_samples);
  } else {
    sampler.sample_from_clusters(clusters, nodes[0]);
  }

  std::unique_ptr<SplittingRule> splitting_rule = splitting_rule_factory->create(
      data, options);

  size_t num_open_nodes = 1;
  size_t i = 0;
  std::vector<std::vector<double>> responses_by_sample(data.get_num_rows(), std::vector<double>(data.get_outcome_index().size(), 0.0));  // std::vector<double> -> std::vector<std::vector<double>> + careful about initialization
  while (num_open_nodes > 0) {
    bool is_leaf_node = split_node(i,
                                   data,
                                   splitting_rule,
                                   sampler,
                                   child_nodes,
                                   nodes,
                                   split_vars,
                                   split_values,
                                   responses_by_sample,
                                   options);
    if (is_leaf_node) {
      --num_open_nodes;
    } else {
      nodes[i].clear();
      ++num_open_nodes;
    }
    ++i;
  }
  
  //std::cout << "survived recursive partitioning" << std::endl;

  std::vector<size_t> drawn_samples;
  sampler.get_samples_in_clusters(clusters, drawn_samples);

  std::unique_ptr<Tree> tree(new Tree(0, child_nodes, nodes,
      split_vars, split_values, drawn_samples, PredictionValues()));

  if (!new_leaf_samples.empty()) {
    //std::cout << "repopulate leaves" << std::endl;
    repopulate_leaf_nodes(tree, data, new_leaf_samples, options.get_honesty_prune_leaves());
  }

  PredictionValues prediction_values;
  if (prediction_strategy != nullptr) {
    //std::cout << "precompute predictions" << std::endl;
    prediction_values = prediction_strategy->precompute_prediction_values(tree->get_leaf_samples(), data);
  }
  //std::cout << "set predictions" << std::endl;
  tree->set_prediction_values(prediction_values);
  //std::cout << "finished building the tree" << std::endl;
  return tree;
}

void TreeTrainer::repopulate_leaf_nodes(const std::unique_ptr<Tree>& tree,
                                        const Data& data,
                                        const std::vector<size_t>& leaf_samples,
                                        const bool honesty_prune_leaves) const {
  size_t num_nodes = tree->get_leaf_samples().size();
  std::vector<std::vector<size_t>> new_leaf_nodes(num_nodes);

  std::vector<size_t> leaf_nodes = tree->find_leaf_nodes(data, leaf_samples);

  for (auto& sample : leaf_samples) {
    size_t leaf_node = leaf_nodes[sample];
    new_leaf_nodes[leaf_node].push_back(sample);
  }
  tree->set_leaf_samples(new_leaf_nodes);
  if (honesty_prune_leaves) {
    tree->honesty_prune_leaves();
  }
}

void TreeTrainer::create_split_variable_subset(std::vector<size_t>& result,
                                               RandomSampler& sampler,
                                               const Data& data,
                                               uint mtry) const {

  // Randomly select an mtry for this tree based on the overall setting.
  size_t num_independent_variables = data.get_num_cols() - data.get_disallowed_split_variables().size();
  size_t mtry_sample = sampler.sample_poisson(mtry);
  size_t split_mtry = std::max<size_t>(std::min<size_t>(mtry_sample, num_independent_variables), 1uL);

  sampler.draw(result,
               data.get_num_cols(),
               data.get_disallowed_split_variables(),
               split_mtry);
}

bool TreeTrainer::split_node(size_t node,
                             const Data& data,
                             const std::unique_ptr<SplittingRule>& splitting_rule,
                             RandomSampler& sampler,
                             std::vector<std::vector<size_t>>& child_nodes,
                             std::vector<std::vector<size_t>>& samples,
                             std::vector<size_t>& split_vars,
                             std::vector<double>& split_values,
                             std::vector<std::vector<double>>& responses_by_sample, // std::vector<double> -> std::vector<std::vector<double>>
                             const TreeOptions& options) const {

  std::vector<size_t> possible_split_vars;
  create_split_variable_subset(possible_split_vars, sampler, data, options.get_mtry());

  bool stop = split_node_internal(node,
                                  data,
                                  splitting_rule,
                                  possible_split_vars,
                                  samples,
                                  split_vars,
                                  split_values,
                                  responses_by_sample,
                                  options.get_min_node_size());
  if (stop) {
    return true;
  }

  size_t split_var = split_vars[node];
  double split_value = split_values[node];

  size_t left_child_node = samples.size();
  child_nodes[0][node] = left_child_node;
  create_empty_node(child_nodes, samples, split_vars, split_values);

  size_t right_child_node = samples.size();
  child_nodes[1][node] = right_child_node;
  create_empty_node(child_nodes, samples, split_vars, split_values);

  // std::cout << split_var << std::endl;
  // std::cout << split_value << std::endl;
  // For each sample in node, assign to left or right child
  // Ordered: left is <= splitval and right is > splitval
  for (auto& sample : samples[node]) {
    if (data.get(sample, split_var) <= split_value) {
      samples[left_child_node].push_back(sample);
    } else {
      samples[right_child_node].push_back(sample);
    }
  }

  // No terminal node
  return false;
}

bool TreeTrainer::split_node_internal(size_t node,
                                      const Data& data,
                                      const std::unique_ptr<SplittingRule>& splitting_rule,
                                      const std::vector<size_t>& possible_split_vars,
                                      const std::vector<std::vector<size_t>>& samples,
                                      std::vector<size_t>& split_vars,
                                      std::vector<double>& split_values,
                                      std::vector<std::vector<double>>& responses_by_sample, // std::vector<double> -> std::vector<std::vector<double>>
                                      uint min_node_size) const {
  // Check node size, stop if maximum reached
  if (samples[node].size() <= min_node_size) {
    split_values[node] = -1.0;
    return true;
  }

  bool stop = relabeling_strategy->relabel(samples[node], data, responses_by_sample);

  if (stop || splitting_rule->find_best_split(data,
                                              node,
                                              possible_split_vars,
                                              responses_by_sample,
                                              samples,
                                              split_vars,
                                              split_values)) {
    split_values[node] = -1.0;
    return true;
  }

  return false;
}

void TreeTrainer::create_empty_node(std::vector<std::vector<size_t>>& child_nodes,
                                    std::vector<std::vector<size_t>>& samples,
                                    std::vector<size_t>& split_vars,
                                    std::vector<double>& split_values) const {
  child_nodes[0].push_back(0);
  child_nodes[1].push_back(0);
  samples.emplace_back();
  split_vars.push_back(0);
  split_values.push_back(0);
}

} // namespace drf
