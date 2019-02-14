// Variable
// type: enum
// name: string
// operand: enum

// Tree_node
// value:
// - number
// - variable, associated with an operand if it's not a leaf
// pointers to two children


// merging trees
// make a list of all Roots
// go through each tree to see if one of the Roots shows up as a child
// if so, do some checks
// - liveness analysis, only merge if Root will be dead
// - ...
// if safe, merge the two trees

// vector<Tree_node*> tree_generation(Context context){
//   // generate a tree for each instruction
//   // create a map of variable name - trees where that variable is a leave
//
//   // call merge_tree
// }
//
// vector<Tree_node*> merge_tree(vector<Tree_node*>){
//   // go through each tree and look at the Root
//   // check the map, and merge trees
//   // delete merged tree
// }
//
// bool check_liveness(Variable v, vector<string> out_set){
//
// }
