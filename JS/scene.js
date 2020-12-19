class Scene{
    constructor(network_canvas, cleanCommands, tagCommands, attrCommands){
		this.network = null;
		this.commandActions = [cleanCommands, tagCommands, attrCommands];
		this.canvas = network_canvas;
		this._deselect();
		this.updateStructure();
	}

	sendCommand(reqKind, reqBody, onResponse = null) {
		const xhr = new XMLHttpRequest();
		if(onResponse !== null){
			xhr.addEventListener('load', ()=>{onResponse(xhr.response);});
		}
		xhr.addEventListener('error', ()=>{ console.log("error"); });
		xhr.open('POST', reqKind);
		xhr.send(reqBody);
	}

	updateStructure() {
		this._deselect();
		this.sendCommand("getJSON", "", (netJSON)=>{
			let options = {
				layout: {
					hierarchical: {
						direction: "UD"
					}
				},
				nodes: {font: {color: '#ffffff'}},
				interaction:{hover:true, selectable: true}	
			};
			const network_JSON = JSON.parse(netJSON);
			this.network = new vis.Network(this.canvas, network_JSON, options);
			
			let this_ref = this;
			this.network.on("click", function (params) {
				params.event = "[original event]";
				this_ref._selectNode(parseInt(params.nodes[0]));
			});
			this.network.on("deselectNode", function (params) {
				this_ref._deselect();
			});
		});
	}

	_selectNode(nodeId) {
		let this_ref = this;
		this.sendCommand("select", nodeId, (nodeType)=>{ 
			if(nodeType === 't'){
				this_ref.commandActions[1];
			}
			else if(nodeType === 'a') {
				this_ref.commandActions[2];
			}
			else {
				console.log("unkown node type received");
				this_ref._deselect();
			}
		});
	}

	_deselect() {
		this.commandActions[0]();
		this.sendCommand("deselect");
	}

}