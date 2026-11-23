We can represent affine transformations with 4x4 matrices by using **homogenous coordinates**.
$$(x,y,z,w)^\top$$
Points are represented with $w=1$ and vectors with $w=0$ (such that they are not affected by translations). An affine transformation $T(p)=Mp+t$ where $M \in \mathbb{R}^{3 \times 3}, t \in \mathbb{R}^3$ of a point $p=(x,y,z)$ can then be written as
$$\begin{pmatrix}
	M & t\\
	0 & 1
\end{pmatrix}
\begin{pmatrix}
	x \\ y \\ z \\ 1
\end{pmatrix}$$
.

### Projections

In the render pipeline, objects have to be transformed from the local model coordinates in to the screen pixel coordinates. To that end, we define different coordinate spaces:
$$
\text{Model} \to \text{World} \to \text{Camera} \to \text{Normalized Device} \to \text{Pixel}
$$
