var bootState = {
    preload: function () {
        game.load.image('progressbar', 'assets/progressBar.png');
    },
    create: function () {
        game.stage.backgroundColor = '#6495ed';
        game.physics.startSystem(Phaser.Physics.ARCADE);

        game.renderer.renderSession.roundPixels = true; 
        game.scale.scaleMode = Phaser.ScaleManager.SHOW_ALL;

        //Imposta dimensione massima
        game.scale.maxWidth = 500;
        game.scale.maxHeight = 251;

        //centro finestra di gioco
        game.scale.pageAlignHorizontally = true;
        game.scale.pageAlignVertically = true;

        game.state.start('load');
    },
};