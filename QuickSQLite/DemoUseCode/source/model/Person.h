//
//  Person.h
//  Quick SQLite Demo
//
//  Created by sudi on 2016/12/20.
//  Copyright © 2016年 quick. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface Person : NSObject
@property (nonatomic, strong) NSString*     name;
@property (nonatomic, assign) CGFloat       height;
@property (nonatomic, strong) UIImage*      avatar;
@property (nonatomic, assign) NSUInteger    age;

@property (nonatomic, assign) NSUInteger    identity;
@end
