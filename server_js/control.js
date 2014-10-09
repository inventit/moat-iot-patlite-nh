var moat = require('moat');
var context = moat.init();
var session = context.session;
var tag = '** Patlite NH Series **';

console.log(tag, 'Start!');

var args = context.clientRequest.dmjob.arguments;
console.log(tag, 'args => ' + JSON.stringify(args));

var nhControllerMapper = session.newModelMapperStub('NHController');
appendNew();

function appendNew() {
  var controller = nhControllerMapper.newModelStub();
  setValues(controller);
  nhControllerMapper.add(controller, {
    success : function(result) {
      control(controller);
    },
    error : function() {
      nhControllerMapper.findByUid('demo', {
        success : function(result) {
          controller = result.array[0];
          control(controller);
        }
      });
    }
  });
}

function setValues(controller) {
  if (args) {
    controller.ipv4Address = args.ipV4Address;
    controller.port = args.port;
    controller.red = args.red;
    controller.yellow = args.yellow;
    controller.green = args.green;
    controller.blue = args.blue;
    controller.white = args.white;
    controller.buzzer = args.buzzer;
    controller.codeJson = args.codeJson;
  }

  controller.ipv4Address = controller.ipv4Address ? controller.ipv4Address : "192.168.1.245";
  controller.port = controller.port ? controller.port : 10000;
  controller.uid = "demo";
}

function control(controller) {
  if (args && args.okGotIt == 'true') {
    controller.okGotIt(session, null, {
      success: function(result) {
        console.log(tag, 'okGotIt => success');
      },
      error : function(errorType, errorCode) {
        console.log(tag, 'okGotIt => error:' + errorType + ':' + errorCode);
      }
    });
  } else {
    controller.apply(session, null, {
      success: function(result) {
        console.log(tag, 'apply => success');
      },
      error : function(errorType, errorCode) {
        console.log(tag, 'apply => error:' + errorType + ':' + errorCode);
      }
    });
  }
}


