var playState = {
    init: function (data) {
        game.stage.backgroundColor = '#6495ed';
        this.tube = game.add.group();
        this.tube.enableBody = true;
        this.coin = game.add.group();
        this.coin.enableBody = true;

        if (data != null) {
            this.map = data.map;
            this.layer = data.layer;
            this.map.setCollisionBetween(1, 34);
            this.map.setCollisionBetween(297, 301);
            this.map.setCollisionBetween(264, 268);
            this.qflower = data.qflower;
            this.qfunghi = data.qfunghi;
            this.tube = data.tube;
            this.brick = data.brick;
            this.koopa = data.koopa;
            this.goomba = data.goomba;
            this.coin = data.coin;
            this.auto = false;
            game.global.life = 1;
        } else this.auto = true;
    },
    create: function () {
        this.wasd = {
            up : game.input.keyboard.addKey(Phaser.Keyboard.W),
            left: game.input.keyboard.addKey(Phaser.Keyboard.A),
            right: game.input.keyboard.addKey(Phaser.Keyboard.D)
        };
        this.fire = game.input.keyboard.addKey(Phaser.Keyboard.X);

        //diciamo di voler usare le freccette
        this.cursor = game.input.keyboard.createCursorKeys();
        game.input.keyboard.addKeyCapture([Phaser.Keyboard.UP, Phaser.Keyboard.DOWN, Phaser.Keyboard.LEFT, Phaser.Keyboard.RIGHT]);
        this.funghi = game.add.group();
        this.funghi.enableBody = true;
        this.funghi.createMultiple(10, 'fungo');
        this.flower = game.add.group();
        this.flower.enableBody = true;
        this.flower.createMultiple(10, 'flower');
        this.brickCoin = game.add.group();
        this.brickCoin.enableBody = true;
        this.brickCoin.createMultiple(10, 'coinBlock');
        this.point();
        if (this.auto) {
            this.createWorld();
        }
        game.world.bringToTop(this.flower);
        game.world.bringToTop(this.funghi);

        game.world.bringToTop(this.qfunghi);
        game.world.bringToTop(this.qflower);
        
        this.bandiera = game.add.sprite(3207, 40, 'bandiera');
        game.physics.arcade.enable(this.bandiera);

        this.bandiera.enableBody = true;
        this.bigMario = false;
        this.FireMario = false;
        this.end = false;
        this.timeChangeToSmall = 0;
        this.fireTime = 0;
        this.player= game.add.sprite(game.camera.x+20, game.height/2, 'player');
        // Tell Phaser that the player will use the  physics engine 
        this.die = false;
        game.physics.arcade.enable(this.player);
        // Add vertical gravity to the player 
        this.player.body.gravity.y = 520;
        this.player.anchor.setTo(0.5, 0.5);
        this.player.animations.add('right', [1,2, 3, 4], 12, true);
        this.player.animations.add('left', [9,10,11], 12, true);
        this.player.animations.add('finish', [4, 5], 15, true);

        this.player.body.linearDamping = 1;
        this.player.body.collideWorldBounds = false;
        game.camera.follow(this.player);
        this.player.frame = 1;
        this.fireBall = game.add.group();
        this.fireBall.enableBody = true;
        this.fireBall.createMultiple(30, 'fireBall');
        this.emitter = game.add.emitter(0,0, 4);
        //set the 'pixel image for the particles
        this.emitter.makeParticles('pixel', 10);
        //settiamo la velocità x e y 
        this.emitter.setYSpeed(-150, 150);
        this.emitter.setXSpeed(-150, 150);
        //setttiamo lo "scale" da 2 a 0 in 800ms
        //i parametri sono: startX, endX, startY, endY, duration
        this.emitter.setScale(2, 0, 2, 0, 800);
        this.emitter.gravity = 500;        

    },
    update: function () {
        //diciamo al Phaser che player e walls collidono
        game.physics.arcade.collide(this.player, [this.layer, this.tube]);
        game.physics.arcade.collide(this.goomba, [this.layer, this.brick, this.question, this.tube]);
        game.physics.arcade.collide(this.koopa, [this.layer, this.brick, this.question, this.tube]);

        game.physics.arcade.collide(this.player, this.questions, this.fun_questions, null, this);
        game.physics.arcade.collide(this.player, this.qfunghi, this.fun_questions, null, this);
        game.physics.arcade.collide(this.player, this.qstar, this.fun_questions, null, this);
        game.physics.arcade.collide(this.player, this.qflower, this.fun_questions, null, this);
        game.physics.arcade.collide(this.player, this.brick, this.fun_brick, null, this);
        game.physics.arcade.collide(this.funghi, [this.qfunghi, this.questions,this.brick,this.layer, this.tube ]);
        //game.physics.arcade.collide(this.funghi, this.questions);
        //game.physics.arcade.collide(this.funghi, this.brick);
        //game.physics.arcade.collide(this.funghi, this.layer);
        //game.physics.arcade.collide(this.player, this.bandiera, this.funBandiera, null, this);
        game.physics.arcade.collide(this.flower, this.qflower);
        game.physics.arcade.collide(this.fireBall,  [this.goomba, this.koopa, this.qfunghi, this.questions,this.brick, this.qstar, this.flower, this.layer, this.tube], this.fun_fire, null, this);
        game.physics.arcade.collide(this.bandiera, this.layer);
        game.physics.arcade.overlap(this.player, this.star, this.takeCoin, null, this);
        game.physics.arcade.overlap(this.player, this.funghi, this.collideFunghi, null, this);
        game.physics.arcade.overlap(this.player, this.flower, this.collideFlower, null, this);        
        //game.physics.arcade.collide(this.player, this.walls);
        //game.physics.arcade.collide(this.enemies, this.walls);
        //(obj1, obj2, callback da chiamare quando c'è overlap, process, context dove eseguire la callback)
        game.physics.arcade.overlap(this.player, this.coin, this.takeCoin, null, this);
        game.physics.arcade.collide(this.player, [this.goomba, this.koopa], this.pVSe, null, this);
        game.physics.arcade.collide(this.goomba, this.koopa, this.eVSe, null, this);
        game.physics.arcade.collide(this.koopa, this.koopa, this.eVSe, null, this);

        
        //se il nextEnemy è passato
        if (!this.player.inWorld && this.player.body.y >= game.height) {
            //this.die = true;
            this.playerDie();
        }
        if (!this.die){
            this.movePlayer();
        } else this.player.body.velocity.x = 0;
        for (let i = 0; i<this.goomba.length; i++) {
            if (game.camera.x >= this.goomba.children[i].body.x -550 && this.goomba.children[i].body.bounce.x != 1) {
                this.addEnemy(this.goomba.children[i]);
            }
        }
        for (let i = 0; i<this.koopa.length; i++) {
            if (game.camera.x >=  this.koopa.children[i].body.x-550 && this.koopa.children[i].body.bounce.x != 1) {
                this.addEnemy(this.koopa.children[i]);
            }
        }
        
        if (this.player.body.x > 3200 ) {
            this.funBandiera();
        }
        if (this.bandiera.body.y >= 124) {
            this.end = true;
            this.player.animations.stop();
            this.player.frame = 1;
            game.time.events.add(1000, this.menu, this);

        }
       
           
    },
    movePlayer: function() {
        // If neither the right or left arrow key is pressed 
         if (((this.cursor.up.isDown || this.wasd.up.isDown)&& (this.player.body.onFloor() || this.player.body.touching.down) )
            && (this.cursor.right.isDown || this.wasd.right.isDown)) { 
            // Move the player upward (jump)
            this.player.animations.stop();
            this.player.frame = 5; 
            this.player.body.velocity.y = -260;
        } 
        else if (((this.cursor.up.isDown || this.wasd.up.isDown)&&  (this.player.body.onFloor() || this.player.body.touching.down) ) && (this.cursor.left.isDown || this.wasd.left.isDown)) { 
            // Move the player upward (jump)
            this.player.animations.stop();
            this.player.frame = 7; 
            this.player.body.velocity.y = -260;
        }
        else if (((this.cursor.up.isDown || this.wasd.up.isDown)&&  (this.player.body.onFloor() || this.player.body.touching.down) )) { 
            // Move the player upward (jump)
            this.player.animations.stop();
            if (this.diretion == -1) {
                this.player.frame = 7;
            } else this.player.frame = 5; 
            this.player.body.velocity.y = -260;
        } 
        else if (this.cursor.right.isDown || this.wasd.right.isDown) { 
            // Move the player to the right 
            if (this.player.body.onFloor() || this.player.body.touching.down) {
                this.player.animations.play('right');                
            } 
            this.diretion = 1;
            this.player.body.velocity.x = 110;
           }
        else if (this.cursor.left.isDown || this.wasd.left.isDown)  { 
             // Move the player to the left 
             if (this.player.body.onFloor() || this.player.body.touching.down) {
                this.player.animations.play('left');                
            } 
            this.diretion = -1;
            this.player.body.velocity.x = -110; 
            }
        else if ( (this.player.body.onFloor() || this.player.body.touching.down) ){ 
            // Stop the player 
            this.player.animations.stop();
            if (this.diretion ==1){
                this.player.frame = 1;
            }
            else if(this.diretion == -1) {
                this.player.frame = 11;
            }
            
            this.player.body.velocity.x = 0;
        }
        else {
            this.player.body.velocity.x = 0;
        }
        //se la freccia verso l'alto è pressata e il player è sul terreno
        if (this.FireMario == true && this.fire.isDown && (game.time.now > this.fireTime +300)) {
            let f = this.fireBall.getFirstDead();
            f.animations.add('fire', [0, 1, 2, 3], 20, true);
            f.animations.play('fire');
            f.outOfBoundsKill = true;
            f.checkWorldBounds = true;
            f.body.gravity.y = 300;
            f.reset(this.player.body.x+ 3, this.player.body.y);
            if (this.diretion == -1 ) {
                f.body.velocity.x = -270;
            } else f.body.velocity.x = 270;
            this.fireTime= game.time.now;
        }

    },
    createquestions: function(question, obj, gid) {
        question.enableBody = true;
        this.map.createFromObjects(obj, gid, 'question', 24, true, false, question);
        question.callAll('animations.add', 'animations', 'spin', [24, 25, 26], 5, question);
        question.callAll('animations.play', 'animations', 'spin');
        question.setAll('body.immovable', true);
    },
    createWorld: function () {
        //create the tilemap
        this.map = game.add.tilemap('map');
        //add the tileset to map
        this.map.addTilesetImage('Mario', 'tileset');
        //create the layer by specifying the name of the Tiled layer
        this.layer = this.map.createLayer('graphics');
        //this.layer.displayHeight = 500;
        //set the world size to match the size of layer
        this.layer.resizeWorld();
        this.layer.wrap = true;
        this.star = game.add.group();
        this.star.enableBody = true;
        this.map.createFromObjects('star', 925, 'star', 0, true, false, this.star);
        /*this.funghi = game.add.group();
        this.funghi.enableBody = true;
        this.map.createFromObjects('funghi', 929, 'fungo', 0, true, false, this.funghi);
        this.funghi.setAll('body.immovable', true);*/
        this.questions = game.add.group();
        this.createquestions(this.questions, 'questions', 25);
        this.qfunghi = game.add.group();
        this.createquestions(this.qfunghi, 'qfunghi', 25);
        this.qstar = game.add.group();
        this.createquestions(this.qstar, 'qstar', 25);
        this.qflower = game.add.group();
        this.createquestions(this.qflower, 'qflower', 25);
        /*this.bandiera = game.add.group();
        this.bandiera.enableBody = true;
        this.map.createFromObjects('bandiera', 314, 'question', 313, true, false, this.bandiera);
        this.map.createFromObjects('bandiera', 1231, 'bandiera', 298, true, false, this.bandiera);
        this.bandiera.setAll('body.immovable',true);*/
        this.map.createFromObjects('coin', 936, 'coinBlock', 1, true, false, this.coin);
        this.coin.callAll('animations.add', 'animations', 'spin', [0, 1], 5, this.coin);
        this.coin.callAll('animations.play', 'animations', 'spin');
        //this.coin.setAll('body.immovable', true);
        this.brick = game.add.group(); 
        this.brick.enableBody = true;
        
        this.map.createFromObjects('brick', 2, 'question', 1, true, false, this.brick);
        this.brick.setAll('body.immovable', true);
        
        
        //enable collissions for the first tilset element (the blue wall)
        // this.map.setCollision(0,true, this.layer, true); 
        this.map.setCollisionBetween(1, 34);
        this.map.setCollisionBetween(297, 301);
        this.map.setCollisionBetween(264, 268);

        this.goomba = game.add.group();
        this.goomba.enableBody = true;
        this.map.createFromObjects('enemies', 930, 'enemies', 1, true, false, this.goomba);  
        this.goomba.setAll('body.bounce.x', 0);
        this.koopa = game.add.group();
        this.koopa.enableBody = true;
        this.map.createFromObjects('koopa', 933, 'koopa', 1, true, false, this.koopa);  
        this.koopa.setAll('body.bounce.x', 0);
        this.koopa.setAll('splash', 0);
        this.koopa.setAll('uccidi', false);

    },
    fun_questions: function (p, b) {
        if (p.body.touching.up && b.frame != 27) {
            b.animations.stop();
            b.frame = 27;
            game.add.tween(b).to({y: b.y -8}, 100, Phaser.Easing.Sinusoidal.In, true, 0, 0, true);
            if (this.qfunghi != null && this.qfunghi.contains(b)) {
                var fungo = this.funghi.getFirstDead();
                fungo.reset(b.x, b.y);
                game.add.tween(fungo).to({y: fungo.y -8}, 100, Phaser.Easing.Sinusoidal.In, true, 0, 0, true);
                game.add.tween(fungo).to({y: fungo.y -16}, 700, null, true, 200, 0, false);
               (function(fungo){
                   setTimeout(function() {
                       fungo.body.velocity.x= 50;
                       fungo.body.gravity.y= 200;
                       fungo.body.bounce.x = 1;
                   }, 1000);                                   
               })(fungo);

            }
            else if (this.qstar != null && this.qstar.contains(b)) {
                let i = this.qstar.getIndex(b);
                console.log('indice '+ i);
                game.add.tween(this.star.children[i]).to({y: b.y -8}, 100, Phaser.Easing.Sinusoidal.In, true, 0, 0, true);
                game.add.tween(this.star.children[i]).to({y: this.star.children[i].y -16}, 1500, null, true, 200, 0, false);
            }
            else if (this.qfunghi != null && this.qflower.contains(b)) {
                var flower = this.flower.getFirstDead();
                flower.reset(b.x, b.y);
                game.add.tween(flower).to({y: flower.y -8}, 100, Phaser.Easing.Sinusoidal.In, true, 0, 0, true);
                game.add.tween(flower).to({y: flower.y -16}, 700, null, true, 200, 0, false);

            }  else {
                let coin = this.brickCoin.getFirstDead();
                coin.reset(b.x, b.y);
                coin.animations.add('coin', [0, 1, 2, 3], 12, false);
                coin.animations.play('coin');
                coin.body.velocity.y -= 400;
                coin.outOfBoundsKill = true;
                this.takeCoin(this.player,  null);
        }
        } 
        this.movePlayer();
    },
    eVSe: function (g, k) {
        if (k.uccidi && k.body.velocity.x != 0) {
            g.kill();
        } else if ( g.uccidi && g.body.velocity.x != 0) {
            k.kill();
        }
    },
    fun_brick: function(p, b) {
        if (p.body.touching.up) {
            if (this.bigMario == true || this.FireMario == true) {
                this.emitter.x = b.x;
                this.emitter.y = b.y;
                this.emitter.start(true, 800, null, 4);
                b.destroy();
            } else game.add.tween(b).to({y: b.y -8}, 100, Phaser.Easing.Sinusoidal.In, true, 0, 0, true);
        }
    },
    fun_fire: function(fire, obj) {
       if (this.goomba.contains(obj) ) {
        fire.body.velocity.x = 0;
        fire.animations.stop()
        fire.frame = 7;
        this.killEnemy(obj);
        fire.kill();
    } else if (this.koopa.contains(obj)){
        obj.kill();
        fire.kill();
    } else if (fire.body.velocity.x == 0){
        fire.kill();    
    } else  fire.body.velocity.y -= 120;
    },
    playerDie: function () {
        if (this.die == false) {
            game.global.life--;
            console.log(game.global.life);
        }
        this.die = true;
        game.input.keyboard.enabled = false;
        this.player.body.velocity.x = 0;
        this.player.body.velocity.y -=50;
        this.player.animations.stop();
        this.player.frame = 6;
        this.goomba.setAll('body.velocity.x', 0);
        this.koopa.setAll('body.velocity.x', 0);
        this.player.body.checkCollision= false;
        game.time.events.add(1000, this.pre_play, this);
    },
    takeCoin: function (player, coin) {
        game.global.score += 1;
        this.coinLabel.text = 'x'+ game.global.score;
        if (coin != null) {
            coin.destroy();
        }
    },
    addEnemy: function (enemy) {
        enemy.body.gravity.y = 500;
        enemy.body.velocity.x = -30;
        //quando il nemico si scontra con il muro, gli fa cambiare direzione
        enemy.body.bounce.x = 1;
        enemy.animations.add('enemy', [0, 1], 5, true);
        enemy.animations.play('enemy');
        //uccide i nemici quando escono dal mondo
        enemy.checkWorldBounds = true; 
        enemy.outOfBoundsKill = true;
    }, 
    collideFunghi: function (p, f) { 
        if (this.FireMario == false) {
            p.loadTexture('bigMario');
            p.body.setSize(16, 32);
            p.anchor.setTo(0, 1);
            this.bigMario = true;
        }
            f.kill();
    },
    collideFlower: function (p, f) {
        if (this.FireMario == false ) {
            p.loadTexture('fireMario');
            p.body.setSize(16, 32);
            p.anchor.setTo(0, 1);
            this.FireMario = true;
            this.bigMario = false;
            f.kill();
        }
    },
    pVSe : function (p, e) {
        if (p.body.touching.down && this.koopa.contains(e)) {
            if (e.splash < game.time.now +200) {
                e.body.velocity.x = 200;
            } else {
                e.loadTexture('splash');
                e.uccidi = true;
                e.body.setSize(16, 16);
                e.frame = 0;
                e.body.velocity.x = 0;
                //e.splash = true;
                e.body.gravity.y = 500;
                e.splash = game.time.now;
                p.body.velocity.y = 32;
                
            }
        }else if (this.koopa.contains(e)) {
            if (e.splash < game.time.now +200 && e.body.velocity.x != 0) {
               if (this.diretion == 1) {
                   e.body.velocity.x = 200;
               } else e.body.velocity.x = -200;
            } else this.playerDie();
        } else if (p.body.touching.down) {
            this.killEnemy(e);
        } else if (this.bigMario == true || this.FireMario == true) {
            this.timeChangeToSmall = game.time.now;
            p.loadTexture('player');
            p.body.setSize(16, 16);
            this.bigMario = false;
            this.FireMario = false;
        }  else if (game.time.now > this.timeChangeToSmall+500 && e.frame != 2) {
            this.playerDie();
        }
    },
    killEnemy: function(e) {
        e.animations.stop();
        e.frame = 2;
        e.body.velocity.x = 0;
        (function(e) {
            setTimeout(() => {
                e.kill();
            }, 100);
        })(e);
    }, 
    funBandiera: function() {
        this.die = true;
        if (this.end == false) {
            //this.player.body.x = b.body.x -16;
            console.log(this.bandiera.body.y)
            this.player.animations.play('finish');
            this.player.body.gravity.y = 0;
            this.player.body.velocity.x=0;
            this.player.body.velocity.y = 50;
            this.bandiera.body.velocity.y = 50;

        }
       
    },
    point : function () {
        this.scoreLabel = game.add.text(10, 10, 'MARIO\n00000000',{
            font: '12px space', 
            fill: '#ffffff'
        });
        this.scoreLabel.fixedToCamera = true;
        game.global.score = 0;
        let c = game.add.sprite(130, 10, 'coinBlock');
        c.fixedToCamera = true;
        this.coinLabel =  game.add.text(150, 10, 'x00',{
            font: '12px space', 
            fill: '#ffffff'
        });
        this.coinLabel.fixedToCamera = true;

        this.world = game.add.text(250, 10, 'WORLD 1-1',{
            font: '12px space', 
            fill: '#ffffff'
        });
        this.world.fixedToCamera = true;
        this.time = game.add.text(370, 10, 'TIME',{
            font: '12px space', 
            fill: '#ffffff'
        });
        this.time.fixedToCamera = true;
    },
   pre_play: function () {
    game.state.start('pre_play');
   },
   menu: function () {
    game.state.start('menu');

   }
};
