# I Invented a Brand New Blending Mode ([video](https://youtu.be/gM-Ja1gioj4?si=76GnDCkQ0mI1tT-4 "I Invented a Brand New Blending Mode. Try it yourself!"))
The blend mode takes values of the base layer, adds the luminosity value of the base layer to it, and subtracts values of the top layer. As described in the video, while trying to recreate standard "Luminosity" blend mode without RGB-to-HSL conversion, I made a mistake. Surprisingly, this "error" created a unique effect.

# $𝑓(a, b) = a + Lum_a - b$
where:
- **$`a`$** is the base layer value;
- **$`Lum_a`$** is the base layer luminance value;
- **$`b`$** is the top layer value.
> [!NOTE]
> To calculate luminance, I used the next formula $`0.2126*R + 0.7152*G + 0.0722*B`$

Its most effective use case involves pairing a base image layer with a uniformly filled top layer representing the base image's average color value. This configuration produces a distinctive visual effect reminiscent of the high-contrast, desaturated aesthetic characteristic of bleach bypass processing (see examples below).

The blend mode is currently only available in my DCTLs for DaVinci Resolve Studio ([FX Trippy DCTL](https://aescripts.com/fx-trippy-dctl/ "FX Trippy DCTL on the aescripts") and [FX Grainny DCTL](https://aescripts.com/fx-grainny-dctl/ "FX Grainny DCTL on the aescripts")) **and** OpenFX plugin, which source code is available [here](OFX-plugin), and is available to download at releases section to the right. It's a hidden gem for color nerds willing to experiment, I hope.

### Support this and future projects:
- BTC: 
```
bc1q44j8e4nws4mjfzgnmajgewcek6ss5q28glfsn2
```
- ETH/USDT/USDC (ERC20): 
```
0x740D1d2fc9DD4E34A3deef142AB53331DC73EAe2
```
- USDT/USDC (TRC20): 
```
TKRYcJsG1Y1yg2zFAarxnW8u81RzMTTcAj
```

## For the End User
**How to download?**  
Find 'Releases' section on the page and there you'll be able to download the plugin.

**How to install?**  
Open archive and copy .bundle file to the following folder:
- Windows version is possible, if I'll see people's interest. ~~Windows: C:\Program Files\Common Files\OFX\Plugins~~
- MacOS: /Library/OFX/Plugins  
> [!NOTE]  
> If it is your first OpenFX plugin and you don't have 'OFX/Plugins' folder, create the folder yourself.


## Examples
![Example 1](Examples/Example1.png)
![Example 2](Examples/Example2.png)
![Example 3](Examples/Example3.png)
