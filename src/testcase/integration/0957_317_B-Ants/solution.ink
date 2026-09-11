// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var q: dynamic = cpp_array(5, 50000);

var sh: dynamic = cpp_array(2500, 2500);

var vis: dynamic = cpp_array(2500, 2500);

func bfs() -> dynamic
{
  var i: dynamic = 0;
  var j: dynamic = 1;
  sh[1000][1000] = n;
  q[0][0] = 1000;
  q[0][1] = 1000;
  while ((i < j))
  {
    var x: dynamic = q[(i % 40000)][0];
    var y: dynamic = q[(i % 40000)][1];
    sh[(x + 1)][y] += (sh[x][y] / 4);
    sh[(x - 1)][y] += (sh[x][y] / 4);
    sh[x][(y + 1)] += (sh[x][y] / 4);
    sh[x][(y - 1)] += (sh[x][y] / 4);
    sh[x][y] %= 4;
    vis[x][y] = 0;
    if (((vis[(x + 1)][y] == 0) && (sh[(x + 1)][y] >= 4)))
    {
      q[(j % 40000)][0] = (x + 1);
      q[(j % 40000)][1] = y;
      j += 1;
      vis[(x + 1)][y] = 1;
    }
    if (((vis[(x - 1)][y] == 0) && (sh[(x - 1)][y] >= 4)))
    {
      q[(j % 40000)][0] = (x - 1);
      q[(j % 40000)][1] = y;
      j += 1;
      vis[(x - 1)][y] = 1;
    }
    if (((vis[x][(y + 1)] == 0) && (sh[x][(y + 1)] >= 4)))
    {
      q[(j % 40000)][0] = x;
      q[(j % 40000)][1] = (y + 1);
      j += 1;
      vis[x][(y + 1)] = 1;
    }
    if (((vis[x][(y - 1)] == 0) && (sh[x][(y - 1)] >= 4)))
    {
      q[(j % 40000)][0] = x;
      q[(j % 40000)][1] = (y - 1);
      j += 1;
      vis[x][(y - 1)] = 1;
    }
    i += 1;
  }
}

func main() -> dynamic
{
  scanf("%d %d", (&n), (&t));
  bfs();
  {
    var i: dynamic = 0;
    while ((i < t))
    {
      var h: dynamic = cpp_uninitialized();
      var g: dynamic = cpp_uninitialized();
      scanf("%d %d", (&h), (&g));
      h += 1000;
      g += 1000;
      if (((((h < 0) || (h > 2000)) || (g < 0)) || (g > 2000)))
      {
        printf("0\n");
      } else
      {
        printf("%d\n", sh[h][g]);
      }
      i += 1;
    }
  }
  getchar();
  getchar();
}
