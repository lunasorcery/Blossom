/* framework header */
#version 430
layout(location = 0) uniform vec4 iResolution;
layout(binding = 0) uniform sampler2D accumulatorTex;




void main()
{
	// readback the buffer
	vec4 tex = texelFetch(accumulatorTex,ivec2(gl_FragCoord.xy),0);

	// divide accumulated color by the sample count
	vec3 color = tex.rgb / tex.a;

	/* perform any post-processing you like here */

	// for example, some B&W with an S-curve for harsh contrast
	//color = smoothstep(0.,1.,color.ggg);

	// present for display
	gl_FragColor = vec4(color,1);
}
