class Scene{
    constructor(network_canvas, cleanCommands, tagCommands, attrCommands){
		this.network = null;
		this.commandActions = [cleanCommands, tagCommands, attrCommands];
		this.canvas = network_canvas;
		this.selectedNode = null;
		this.sendCommand("getJSON", null);
	}

	sendCommand(comandName, comandBody, onResponse = null) {
		const xhr = new XMLHttpRequest();
		if(onResponse === null){
			let this_ref = this;
			xhr.addEventListener('load', ()=>{this_ref._updateStructure(xhr.response);});
		}
		else{
			xhr.addEventListener('load', ()=>{onResponse(xhr.response);});
		}
		xhr.addEventListener('error', ()=>{ console.log("error"); });
		xhr.open('POST', 'http://localhost:64000/' + comandName);
		xhr.send( JSON.stringify(comandBody) );
	}

	_updateStructure(newJSON) {
		let options = {
			layout: {
				hierarchical: {
					direction: "UD"
				}
			},
			nodes: {font: {color: '#ffffff'}},
			interaction:{hover:true, selectable: true}	
		};
		const network_JSON = JSON.parse(newJSON);
		this.network = new vis.Network(this.canvas, network_JSON, options);
		
		let this_ref = this;
		this.network.on("click", (params) => {
			params.event = "[original event]";
			this.selectedNode =  params.nodes[0];
			this.sendCommand("getNodeType", { "entity": this.selectedNode.toString() }, (nodeType) => {
				if (nodeType === '\"tag\"'){
					this_ref.commandActions[1]();
				}
				else if (nodeType === '\"attribute\"') {
					this_ref.commandActions[2]();
				}
				else {
					console.log("unkown node type received");
					this_ref._deselect();
				}
			});
		});
		this.network.on("deselectNode", (params) => {
			this_ref._deselect();
		});
		this._deselect();
	}

	_deselect() {
		this.commandActions[0]();
		this.selectedNode = null;
	}

}