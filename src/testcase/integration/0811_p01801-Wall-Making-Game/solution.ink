// Translated from solution.cpp.

func syosu(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.");
}

var inf: dynamic = (1 << 30);

var INF: dynamic = (1 << 60);

var pi: dynamic = acos(-1);

var eps: dynamic = 1e-8;

var mod: dynamic = (1e9 + 7);

var dx: dynamic = [-1, 0, 1, 0];

var dy: dynamic = [0, -1, 0, 1];

var M: dynamic = 21;

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(M, M, M, M);

func Rec(lx: dynamic, rx: dynamic, ly: dynamic, ry: dynamic) -> dynamic
{
  if (((lx == rx) || (ly == ry)))
  {
    return 0;
  }
  if ((dp[lx][rx][ly][ry] >= 0))
  {
    return dp[lx][rx][ly][ry];
  }
  var st: dynamic = cpp_uninitialized();
  {
    var i: dynamic = lx;
    while ((i < rx))
    {
      {
        var j: dynamic = ly;
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
    var i: dynamic = 0;
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

func main() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      {
        var j: dynamic = 0;
        while ((j < M))
        {
          {
            var k: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < h))
    {
      read(a[i]);
      i += 1;
    }
  }
  write(( (Rec(0, h, 0, w)) ? "First" : "Second"), "\n");
}
