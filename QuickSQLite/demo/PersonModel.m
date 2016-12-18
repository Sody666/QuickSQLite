//
//  PersonModel.m
//  QuickSQLiteDemo
//
//  Created by sudi on 2016/12/16.
//  Copyright © 2016年 quick. All rights reserved.
//

#import "PersonModel.h"

@implementation PersonModel
-(NSString*)description{
    NSString* result = [NSString stringWithFormat:@"Name: %@  Age: %d Identity: %d", self.name, self.age, self.identity];
    
    return result;
}
@end
