var createState = {
   
    
    create: function () {
        game.stage.backgroundColor = '#6495ed';
        this.marker;
        this.currentTile;
        this.disableTile = false;
        this.pressD = false;
        //game.stage.backgroundColor = '#3498db';
        this.map = game.add.tilemap('createmap');
        //add the tileset to map
        this.map.addTilesetImage('Mario', 'tileset');
        this.currentTile = this.map.getTile(1, 1);
        //create the layer by specifying the name of the Tiled layer
        this.layer = this.map.createLayer('graphics');
        this.layer.resizeWorld();
        this.marker = game.add.graphics();
        this.marker.lineStyle(2, 0x000000, 1);
        this.marker.drawRect(0, 0, 16, 16);
        game.input.keyboard.enabled = true;
        this.cursors = game.input.keyboard.createCursorKeys();
        game.global.life = 0;
        //this.qfunghi = game.add.group ();
        //this.qfunghi.inputEnableChildren = true;
        //this.qfunghi.enableBody=true;
        //this.qflower = game.add.group ();
        //this.qflower.inputEnableChildren = true;
        //this.tube = game.add.group();
        //this.tube.inputEnableChildren = true;
        this.qfunghi = game.add.group();
        this.qfunghi.inputEnableChildren = true;
        this.qfunghi.enableBody=true;
        this.qflower = game.add.group();
        this.qflower.inputEnableChildren = true;
        this.qflower.enableBody = true;
        this.tube = game.add.group();
        this.tube.inputEnableChildren = true;
        this.tube.enableBody = true;
        this.brick = game.add.group();
        this.brick.inputEnableChildren = true;
        this.brick.enableBody = true;
        this.koopa = game.add.group();
        this.koopa.inputEnableChildren = true;
        this.koopa.enableBody = true; 
        this.goomba = game.add.group();
        this.goomba.inputEnableChildren = true;
        this.goomba.enableBody = true;        
        this.coin = game.add.group();
        this.coin.inputEnableChildren = true;
        this.coin.enableBody = true;
        this.button1 = game.add.button(10, 10, 'fungo', this.addFungo, this, 2, 1, 0);
        this.button1.fixedToCamera = true;
        this.button2 = game.add.button(10, 30, 'flower', this.addFlower, this, 2, 1, 0);
        this.button2.fixedToCamera = true;
        this.button = game.add.button(10, 50, 'tubebutton', this.addTube, this, 64, 64, 64);
        this.button.fixedToCamera = true;
        this.button3 = game.add.button(10, 70, 'question', this.addBrick, this, 2, 2, 2);
        this.button3.fixedToCamera = true;
        this.button4 = game.add.button(430, 10, 'koopa', this.addKopa, this, 1, 1, 1);
        this.button4.fixedToCamera = true;
        this.button5 = game.add.button(450, 10, 'enemies', this.addGoomba, this, 1, 1, 1);
        this.button5.fixedToCamera = true;
        this.button6 = game.add.button(30, 10, 'coinBlock', this.addCoin, this, 1, 1, 1);
        this.button6.fixedToCamera = true;
        this.buttonStart = game.add.button(10, 90, 'player', this.actionOnClick, this, 1 ,1 ,1);
        this.buttonStart.fixedToCamera = true;
        //this.button1.onInputOver.add(this.over, this);
        this.button1.onInputOut.add(this.out, this);
        this.button1.onInputUp.add(this.up, this);
        this.button2.onInputOver.add(this.over, this);
        this.button2.onInputOut.add(this.out, this);
        this.button2.onInputUp.add(this.up, this);
    },
    update: function () {
        this.marker.x = this.layer.getTileX(game.input.activePointer.worldX)* 16;
        this.marker.y = this.layer.getTileY(game.input.activePointer.worldY)* 16;
        if (game.input.keyboard.isDown(Phaser.Keyboard.D)) {
            this.pressD = true;
            console.log(this.pressD);
        }
        if (!game.input.keyboard.isDown(Phaser.Keyboard.D)) {
            if (this.pressD) {
                this.pressD = false;
                if (this.disableTile) {
                this.disableTile = false;
                } else this.disableTile = true;
                console.log(this.disableTile);
            }
        }
        if (game.input.mousePointer.isDown) {
            if (game.input.keyboard.isDown(Phaser.Keyboard.SHIFT)) {
                this.currentTile = this.map.getTile(this.layer.getTileX(this.marker.x), this.layer.getTileY(this.marker.y));
            }
            else {
                //if  (this.map.getTile(this.layer.getTileX(this.marker.x), this.layer.getTileY(this.marker.y)).index != this.currentTile){
                if ( !this.disableTile){
                    this.map.putTile(this.currentTile, this.layer.getTileX(this.marker.x), this.layer.getTileY(this.marker.y), this.layer);
                }    
                //}
            }
        }
        if (this.cursors.left.isDown) {
            game.camera.x -= 4;
        }
        else if (this.cursors.right.isDown) {
            game.camera.x += 4;
            console.log(game.camera.x);
        }

        if (this.cursors.up.isDown) {
            game.camera.y -= 4;
        }
        else if (this.cursors.down.isDown) {
            game.camera.y += 4;
        }
    },
    up: function () {
        console.log('button up', arguments);
    },
    
    over: function () {
        console.log('button over');

    },
    out: function () {
        console.log('button out');
    },
    addTube: function () {
        let clone = game.add.sprite(game.camera.x +100, game.height/2, 'tube',64, this.tube);
        clone.inputEnabled = true;
        clone.input.enableDrag();
    },
    addFungo: function () {
        let clone;
        if (this.qfunghi.length == 0) {
            clone = game.add.sprite(game.camera.x +100, game.height/2, 'question',24, this.qfunghi);            
        }
        else {
            clone = game.add.sprite(this.qfunghi.children[this.qfunghi.length-1].x+16,this.qfunghi.children[this.qfunghi.length-1].y , 'question',24, this.qfunghi); 
        }
        clone.inputEnabled = true;
        clone.input.enableDrag();
        clone.animations.add('question', [24, 25, 26], 5, true);
        clone.animations.play('question');
      //clone.input.startDrag(game.input.activePointer);
      
    },
    addCoin: function () {
        let clone;
        if (this.coin.length == 0) {
            clone = game.add.sprite(game.camera.x +100, game.height/2, 'coinBlock', 1,this.coin);

        }
        else {
            clone = game.add.sprite(this.coin.children[this.coin.length-1].x+16,this.coin.children[this.coin.length-1].y , 'coinBlock', 1,this.coin); 
        }
        clone.animations.add('coin', [0, 1], 5, true);
        clone.animations.play('coin');
        clone.inputEnabled = true;
        clone.input.enableDrag();
    },
    addFlower: function () {
    
        let clone;
        if (this.qflower.length == 0) {
            clone = game.add.sprite(game.camera.x +100, game.height/2, 'question', 24,this.qflower);
        } else clone = game.add.sprite(this.qflower.children[this.qflower.length-1].x+16,this.qflower.children[this.qflower.length-1].y , 'question', 24,this.qflower);
        clone.inputEnabled = true;
        clone.input.enableDrag();
        clone.animations.add('question', [24, 25, 26], 5, true);
        clone.animations.play('question');
        //clone.input.startDrag(game.input.activePointer);
      
      },
      addBrick: function () {
        let clone;
        if (this.brick.length == 0 ) {
            clone = game.add.sprite(game.camera.x +100, game.height/2, 'question', 1 ,this.brick);
        } else clone = game.add.sprite(this.brick.children[this.brick.length-1].x+16,this.brick.children[this.brick.length-1].y , 'question', 1,this.brick);
            clone.inputEnabled = true;
        clone.input.enableDrag();
      },
      addKopa: function () {
        let clone = game.add.sprite(game.camera.x +100, game.height/2, 'koopa', 1 ,this.koopa);
        clone.inputEnabled = true;
        clone.input.enableDrag();
      },
      addGoomba: function () {
        let clone = game.add.sprite(game.camera.x +100, game.height/2, 'enemies', 1 ,this.goomba);
        clone.inputEnabled = true;
        clone.input.enableDrag();
      },
      actionOnClick: function () {
          /*this.walls.inputEnableChildren = false;
          for (let i = 0; i<this.walls.lenght; i++) {
            this.walls.children[i].inputEnabled = false;
              this.walls.children[i].input.disableDrag();
              this.walls.children[i].draggable = false;
          }*/
          this.button1.destroy();
          this.button.destroy();
          this.button2.destroy();
          this.button3.destroy();
          this.button4.destroy();
          this.button5.destroy();
          this.button6.destroy();
          this.buttonStart.destroy();
          this.marker.destroy();
          //var middleTop = game.add.sprite(100, 80, 'wallH', 0, this.walls); middleTop.scale.setTo(1.5, 1); 
          //var middleBottom = game.add.sprite(100, 240, 'wallH', 0, this.walls); middleBottom.scale.setTo(1.5, 1);
          this.qflower.setAll('body.immovable', true);
          this.qfunghi.setAll('body.immovable', true);
          this.tube.setAll('body.immovable', true);
          this.brick.setAll('body.immovable', true);
          this.koopa.setAll('body.bounce.x', 0);
          this.koopa.setAll('splash', 0);
          this.goomba.setAll('body.bounce.x', 0);

          game.state.start('play', false, false, {map: this.map, layer: this.layer,tube: this.tube, coin: this.coin, qflower: this.qflower, qfunghi: this.qfunghi, brick: this.brick, koopa: this.koopa, goomba: this.goomba});
      }
}

