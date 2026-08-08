// Translated from solution.cpp.

func syosu(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

var inf = (1 << 30);

var INF = (1 << 60);

var pi = acos(-1);

var eps = 1e-8;

var mod = (1e9 + 7);

var dx = [-1, 0, 1, 0];

var dy = [0, -1, 0, 1];

var M = 21;

var h: dynamic;

var w: dynamic;

var a: dynamic;

var dp = cpp_array(M, M, M, M);

func Rec(lx: dynamic, rx: dynamic, ly: dynamic, ry: dynamic)
{
  if (((lx == rx) || (ly == ry)))
  {
    return 0;
  }
  if ((dp[lx][rx][ly][ry] >= 0))
  {
    return dp[lx][rx][ly][ry];
  }
  var st: dynamic;
  {
    var i = lx;
    while ((i < rx))
    {
      {
        var j = ly;
        while ((j < ry))
        {
          if ((a[i][j] == cpp_char(".")))
          {
            st.insert((((Rec(lx, i, ly, j) ^ Rec(lx, i, (j + 1), ry)) ^ Rec((i + 1), rx, ly, j)) ^ Rec((i + 1), rx, (j + 1), ry)));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while (true)
    {
      if ((st.find(i) == st.end()))
      {
        dp[lx][rx][ly][ry] = i;
        break;
      }
      i += 1;
    }
  }
  return dp[lx][rx][ly][ry];
}

func main()
{
  {
    var i = 0;
    while ((i < M))
    {
      {
        var j = 0;
        while ((j < M))
        {
          {
            var k = 0;
            while ((k < M))
            {
              fill(dp[i][j][k], (dp[i][j][k] + M), -1);
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  read(h, w);
  a = vs(h);
  {
    var i = 0;
    while ((i < h))
    {
      read(a[i]);
      i += 1;
    }
  }
  write((if (Rec(0, h, 0, w)) "First" else "Second"), "\n");
}
