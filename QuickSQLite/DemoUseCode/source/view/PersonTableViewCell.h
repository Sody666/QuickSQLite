//
//  PersonTableViewCell.h
//  Quick SQLite Demo
//
//  Created by sudi on 2016/12/20.
//  Copyright © 2016年 quick. All rights reserved.
//

#import <UIKit/UIKit.h>

@class Person;
@interface PersonTableViewCell : UITableViewCell
-(void)fillCellWithPerson:(Person*)person;
-(id)initWithReuseIdentifier:(NSString*)identifier;
@end
