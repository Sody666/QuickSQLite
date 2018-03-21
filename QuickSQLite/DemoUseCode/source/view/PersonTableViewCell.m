//
//  PersonTableViewCell.m
//  Quick SQLite Demo
//
//  Created by sudi on 2016/12/20.
//  Copyright © 2016年 quick. All rights reserved.
//

#import "PersonTableViewCell.h"
#import "Person.h"

@interface PersonTableViewCell()
@property (nonatomic, weak) UILabel* labelName;
@property (nonatomic, weak) UILabel* labelAge;
@property (nonatomic, weak) UILabel* labelHeight;
@property (nonatomic, weak) UIImageView* imageViewAvatar;
@end

@implementation PersonTableViewCell

-(id)init{
    self = [super init];
    if (self) {
        [self setupWidgets];
    }
    
    return self;
}

-(id)initWithReuseIdentifier:(NSString*)identifier{
    self = [super initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier];
    if (self) {
        [self setupWidgets];
    }
    
    return self;
}

-(void)setupWidgets{    
    [QLayoutManager layoutForFileName:@"PersonTableViewCell" entrance:self holder:self];
}

-(void)fillCellWithPerson:(Person*)person{
    self.labelName.text = person.name;
    self.labelAge.text = @(person.age).stringValue;
    self.labelHeight.text = @(person.height).stringValue;
    self.imageViewAvatar.image = person.avatar;
    
    [self setNeedsLayout];
    [self layoutIfNeeded];
}
@end
