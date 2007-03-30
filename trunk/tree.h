/*
 * file:	tree.h
 * purpose:	to manhandle a tree structure. it is to store an unsigned int
 * 		this code so wasn't worth it. 
 * created:	03-12-2007
 * creator:	rian shelley
 */

/* all functions are declared as inline, so need to be in the header file */

#ifndef TREE_H
#define TREE_H

/* this timeout defines out long we send packets to a client when it asks us */
/* this is given in milliseconds */
#define TIMEOUT 60000

#include "mygettime.h"

typedef struct t_treenode {
	unsigned int ip;
	unsigned long long timeout;
	struct t_treenode* left;
	struct t_treenode* right;
} treenode;


/*
 * function:	addnode()
 * purpose:	to add (or update) an ip address in the tree
 * recieves:	a pointer to a node, and the ip to add
 */
inline void addnode(treenode** node, unsigned int ip) {
	treenode* root = *node;
	/* base case */
	if (!*node) {
		*node = (treenode*)malloc(sizeof(treenode));
		(*n)->left = 0;
		(*n)->right = 0;
		(*n)->timeout = gettime() + TIMEOUT;
		(*)n->ip = ip;
	} else if (root->ip == ip) {
		timeout = gettime() + TIMEOUT;
	} else if (root->ip > ip) {
		if (root->left) 
			addnode(root->left, ip);
		else {
			treenode* n = (treenode*)malloc(sizeof(treenode));
			n->left = 0;
			n->right = 0;
			n->timeout = gettime() + TIMEOUT;
			n->ip = ip;
			root->left = n;
		}
	} else {
		if (root->right) 
			addnode(root->right, ip);
		else {
			treenode* n = (treenode*)malloc(sizeof(treenode));
			n->left = 0;
			n->right = 0;
			n->timeout = gettime() + TIMEOUT;
			n->ip = ip;
			root->right = n;
		}
	}
}

/*
 * function:	cleantree()
 * purpose:	to prune nodes out of the tree with expired timers
 * recieves:	a pointer to the address of the root node (it may change what
 * 		the root node is, if the root node is expired)
 */
/* 
 * not implemented for now, may do this elsewhere
 
void cleantree(treenode**root) 
{


}
*/

/*
 * function:	removenode()
 * purpose:	to remove a node from the tree
 * recieves:	the ip to remove
 */
void removenode(treenode** n) {
	treenode* tmp = *n;
	if (!(*n)->left) {
		*n = (*n)->right;
		free(tmp);
	} else if (!(*n)->right) {
		*n = (*n)->left;
		free(tmp);
	} else {
		/* find inorder predecessor */
		treenode* tmp2 = 0;
		tmp = (*node)->left;
		while(tmp->right) {
			tmp2 = tmp;
			tmp = tmp->right;
		}
		(*node)->ip = tmp->ip;
		(*node)->timeout = tmp->timeout;
		/* recursively delete the predecessor */
		removenode(&tmp);
	}
}

#endif /* TREE_H */
