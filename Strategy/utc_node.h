//
//  node.h
//  Strategy
//
//  Created by chenzm on 2020/4/24.
//  Copyright © 2020 Yongfeng Zhang. All rights reserved.
//

#ifndef utc_node_h
#define utc_node_h

struct UCT_Node {
    UCT_Node* children[MAX_N];
    UCT_Node* parent;
    float score;  //
    float searched_times;
};

#endif /* node_h */
