//
//  types.h
//  Strategy
//
//  Created by chenzm on 2020/4/25.
//  Copyright © 2020 Yongfeng Zhang. All rights reserved.
//

#ifndef types_h
#define types_h

enum Player {
    me_=2, //
    you_=1 //
};
enum Board {
    empty=0,
    me=2,
    you=1,
    no=3
};
enum Result {
    win_loss = 1,
    tie = 2,
    not_end = 3,
};


#endif /* types_h */
