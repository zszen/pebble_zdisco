//z weather simple
module.exports = 
[
	{//info
		"type": "heading", 
		"defaultValue": "Zszen Disco Watch" 
	}, 
	{//info
		"type": "text", 
		"defaultValue": "config v 1.0" 
	},
	{
		"type": "section",
		"items": [
			{
				"type": "select",
				"messageKey": "CONFIG_theme",
				"defaultValue": "0",
				"label": "Theme",
				"options": [
					{ 
						"label": "White & Colorful",
						"value": "0" 
					},
					{ 
						"label": "Black & Colorful",
						"value": "1" 
					},
					{ 
						"label": "Black & White",
						"value": "2" 
					},
					{ 
						"label": "White & Black",
						"value": "3" 
					},
					{ 
						"label": "White & Blue",
						"value": "4" 
					},
					{ 
						"label": "Custom",
						"value": "5" 
					}
				]
			},
			{
				"type": "color",
				"messageKey": "CONFIG_bgColor",
				"defaultValue": "0x000000",
				"label": "Background Color (Custom)",
				"sunlight": false,
				"allowGray": true
			},
			{
				"type": "color",
				"messageKey": "CONFIG_hourColor",
				"defaultValue": "0xffffff",
				"label": "Hour Color (Custom)",
				"sunlight": false,
				"allowGray": true
			},
			{
				"type": "color",
				"messageKey": "CONFIG_minuteColor",
				"defaultValue": "0xffffff",
				"label": "Minute Color (Custom)",
				"sunlight": false,
				"allowGray": true
			},
			{
				"type": "color",
				"messageKey": "CONFIG_gridColor",
				"defaultValue": "0xffffff",
				"label": "Grid Line Color (Custom)",
				"sunlight": false,
				"allowGray": true
			},
			{//submit button
				"type": "submit",
				"defaultValue": "Save & Restart"
			}
		]
	}
];
