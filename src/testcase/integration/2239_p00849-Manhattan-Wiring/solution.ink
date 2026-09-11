// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, -1, 0, 1];

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var B: dynamic = cpp_array(9, 9);

var x2: dynamic = cpp_uninitialized();

var y2: dynamic = cpp_uninitialized();

var x3: dynamic = cpp_uninitialized();

var y3: dynamic = cpp_uninitialized();

func bfs(x: dynamic, y: dynamic, tar: dynamic) -> dynamic
{
  var d: dynamic = cpp_array(9, 9);
  memset(d, -1, cpp_sizeof(d));
  d[y][x] = 0;
  var Q: dynamic = cpp_array(81);
  var head: dynamic = 0;
  var tail: dynamic = 0;
  Q[cpp_update(tail, "++")] = ((y * w) + x);
  while ((head < tail))
  {
    var i: dynamic = (Q[head] / w);
    var j: dynamic = (Q[head] % w);
    head += 1;
    rep(k, 4);
    {
      var yy: dynamic = (i + dy[k]);
      var xx: dynamic = (j + dx[k]);
      if ((((((0 <= yy) && (yy < h)) && (0 <= xx)) && (xx < w)) && (d[yy][xx] == -1)))
      {
        if ((B[yy][xx] == tar))
        {
          return (d[i][j] + 1);
        }
        if ((B[yy][xx] == 0))
        {
          Q[cpp_update(tail, "++")] = ((yy * w) + xx);
          d[yy][xx] = (d[i][j] + 1);
        }
      }
    }
  }
  return 777;
}

var ans: dynamic = cpp_uninitialized();

func dfs(x: dynamic, y: dynamic, now: dynamic) -> dynamic
{
  var hstar: dynamic = (bfs(x, y, 2) + bfs(x3, y3, 3));
  if ((ans < ((now + 1) + hstar)))
  {
    return;
  }
  var cnt: dynamic = 0;
  rep(k, 4);
  {
    var xx: dynamic = (x + dx[k]);
    var yy: dynamic = (y + dy[k]);
    if ((((((0 <= xx) && (xx < w)) && (0 <= yy)) && (yy < h)) && (B[yy][xx] == 7)))
    {
      cnt += 1;
    }
  }
  if ((cnt >= 2))
  {
    return;
  }
  {
    var L: dynamic = 3;
    while ((L <= 8))
    {
      var ng: dynamic = cpp_uninitialized();
      if (((y > 0) && (((x + L) - 1) < w)))
      {
        ng = true;
        if (ng)
        {
          if ((B[(y - 1)][(x + j)] != 7))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[y][(x + j)] != ( (((j == 0) || (j == (L - 1)))) ? 7 : 0)))
          {
            ng = false;
            break;
          }
        }
        if ((ng && ((y + 1) < h)))
        {
          if ((((0 < j) && (j < (L - 1))) && (B[(y + 1)][(x + j)] != 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          return;
        }
        ng = true;
        if ((ng && ((y - 2) >= 0)))
        {
          if ((((0 < j) && (j < (L - 1))) && (B[(y - 2)][(x + j)] != 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[(y - 1)][(x + j)] != ( (((j == 0) || (j == (L - 1)))) ? 7 : 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[y][(x + j)] != 7))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          return;
        }
      }
      if (((y > 0) && (((x - L) + 1) >= 0)))
      {
        ng = true;
        if (ng)
        {
          if ((B[(y - 1)][(x - j)] != 7))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[y][(x - j)] != ( (((j == 0) || (j == (L - 1)))) ? 7 : 0)))
          {
            ng = false;
            break;
          }
        }
        if ((ng && ((y + 1) < h)))
        {
          if ((((0 < j) && (j < (L - 1))) && (B[(y + 1)][(x - j)] != 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          return;
        }
        ng = true;
        if ((ng && ((y - 2) >= 0)))
        {
          if ((((0 < j) && (j < (L - 1))) && (B[(y - 2)][(x - j)] != 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[(y - 1)][(x - j)] != ( (((j == 0) || (j == (L - 1)))) ? 7 : 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[y][(x - j)] != 7))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          return;
        }
      }
      if (((((y + L) - 1) < h) && ((x + 1) < w)))
      {
        ng = true;
        if (ng)
        {
          if ((B[(y + i)][x] != 7))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[(y + i)][(x + 1)] != ( (((i == 0) || (i == (L - 1)))) ? 7 : 0)))
          {
            ng = false;
            break;
          }
        }
        if ((ng && ((x + 2) < w)))
        {
          if ((((0 < i) && (i < (L - 1))) && (B[(y + i)][(x + 2)] != 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          return;
        }
        ng = true;
        if ((ng && ((x - 1) >= 0)))
        {
          if ((((0 < i) && (i < (L - 1))) && (B[(y + i)][(x - 1)] != 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[(y + i)][x] != ( (((i == 0) || (i == (L - 1)))) ? 7 : 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[(y + i)][(x + 1)] != 7))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          return;
        }
      }
      if (((((y - L) + 1) >= 0) && ((x + 1) < w)))
      {
        ng = true;
        if (ng)
        {
          if ((B[(y - i)][x] != 7))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[(y - i)][(x + 1)] != ( (((i == 0) || (i == (L - 1)))) ? 7 : 0)))
          {
            ng = false;
            break;
          }
        }
        if ((ng && ((x + 2) < w)))
        {
          if ((((0 < i) && (i < (L - 1))) && (B[(y - i)][(x + 2)] != 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          return;
        }
        ng = true;
        if ((ng && ((x - 1) >= 0)))
        {
          if ((((0 < i) && (i < (L - 1))) && (B[(y - i)][(x - 1)] != 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[(y - i)][x] != ( (((i == 0) || (i == (L - 1)))) ? 7 : 0)))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          if ((B[(y - i)][(x + 1)] != 7))
          {
            ng = false;
            break;
          }
        }
        if (ng)
        {
          return;
        }
      }
      L += 1;
    }
  }
  rep(k, 4);
  {
    var xx: dynamic = (x + dx[k]);
    var yy: dynamic = (y + dy[k]);
    if (((((0 <= xx) && (xx < w)) && (0 <= yy)) && (yy < h)))
    {
      if ((B[yy][xx] == 2))
      {
        ans = ((now + 1) + bfs(x3, y3, 3));
        return;
      } else if ((B[yy][xx] == 0))
      {
        B[yy][xx] = 7;
        dfs(xx, yy, (now + 1));
        B[yy][xx] = 0;
      }
    }
  }
}

func main() -> dynamic
{
  while (cpp_comma(scanf("%d%d", (&h), (&w)), h))
  {
    cpp_statement("rep(i,h) rep(j,w) scanf(\"%d\",B[i]+j); rep(i,h)");
    B[y2][x2] = 7;
    ans = 777;
    dfs(x2, y2, 0);
    printf("%d\n",  ((ans < 777)) ? ans : 0);
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((B[i][j] == 2))
      {
        x2 = j;
        y2 = i;
      }
      if ((B[i][j] == 3))
      {
        x3 = j;
        y3 = i;
      }
    }
