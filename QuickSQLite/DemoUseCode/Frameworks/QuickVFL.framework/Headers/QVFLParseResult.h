//
//  QVFLParseResult.h
//  QuickVFL2
//
//  Created by 苏第 on 17/12/1.
//  Copyright © 2017年 Quick. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface QVFLParseResult : NSObject
@property (nonatomic, strong) NSArray* constraints;
@property (nonatomic, strong) NSDictionary* namedConstraints;
@end
