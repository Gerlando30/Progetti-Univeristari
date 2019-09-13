var pre_playState = {
    create: function () {
        game.stage.backgroundColor = '#000000';
        this.scoreLabel = game.add.text(10, 10, 'MARIO\n00000000',{
            font: '12px space', 
            fill: '#ffffff'
        });
        this.scoreLabel.fixedToCamera = true;
        game.world.bringToTop(this.scoreLabel);
        //inizializziamo la variabile score a 0
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
        this.life = game.add.text(game.camera.x+250, game.height/2, 'x',{
            font: '12px space', 
            fill: '#ffffff'
        });
        this.mario = game.add.sprite( game.camera.x + 230, game.height/2, 'player');
        this.mario.frame = 1;
        if (game.global.life <= 0) {
            game.time.events.add(1000, this.startMenu, this);
            game.global.life = 3;
            this.life.text = 'GAME OVER';
        } else {
            game.time.events.add(1000, this.restart, this);
            this.life.text = 'x'+ game.global.life
        }

        game.world.bringToTop(this.scoreLabel);
        game.world.bringToTop(this.time);
        game.world.bringToTop(this.coinLabel);
    }, 
    startMenu: function () {
        game.state.start('menu');
    }, 
    restart: function () {
        game.input.keyboard.enabled = true;
        game.state.start('play');

    }
}