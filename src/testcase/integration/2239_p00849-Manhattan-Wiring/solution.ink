// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var dx = [1, 0, -1, 0];

var dy = [0, -1, 0, 1];

var h: dynamic;

var w: dynamic;

var B = cpp_array(9, 9);

var x2: dynamic;

var y2: dynamic;

var x3: dynamic;

var y3: dynamic;

func bfs(x: dynamic, y: dynamic, tar: dynamic)
{
  var d = cpp_array(9, 9);
  memset(d, -1, cpp_sizeof(d));
  d[y][x] = 0;
  var Q = cpp_array(81);
  var head = 0;
  var tail = 0;
  Q[cpp_update(tail, "++")] = ((y * w) + x);
  while ((head < tail))
  {
    var i = (Q[head] / w);
    var j = (Q[head] % w);
    head += 1;
    rep(k, 4);
    {
      var yy = (i + dy[k]);
      var xx = (j + dx[k]);
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

var ans: dynamic;

func dfs(x: dynamic, y: dynamic, now: dynamic)
{
  var hstar = (bfs(x, y, 2) + bfs(x3, y3, 3));
  if ((ans < ((now + 1) + hstar)))
  {
    return;
  }
  var cnt = 0;
  rep(k, 4);
  {
    var xx = (x + dx[k]);
    var yy = (y + dy[k]);
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
    var L = 3;
    while ((L <= 8))
    {
      var ng: dynamic;
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
          if ((B[y][(x + j)] != (if (((j == 0) || (j == (L - 1)))) 7 else 0)))
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
          if ((B[(y - 1)][(x + j)] != (if (((j == 0) || (j == (L - 1)))) 7 else 0)))
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
          if ((B[y][(x - j)] != (if (((j == 0) || (j == (L - 1)))) 7 else 0)))
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
          if ((B[(y - 1)][(x - j)] != (if (((j == 0) || (j == (L - 1)))) 7 else 0)))
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
          if ((B[(y + i)][(x + 1)] != (if (((i == 0) || (i == (L - 1)))) 7 else 0)))
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
          if ((B[(y + i)][x] != (if (((i == 0) || (i == (L - 1)))) 7 else 0)))
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
          if ((B[(y - i)][(x + 1)] != (if (((i == 0) || (i == (L - 1)))) 7 else 0)))
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
          if ((B[(y - i)][x] != (if (((i == 0) || (i == (L - 1)))) 7 else 0)))
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
    var xx = (x + dx[k]);
    var yy = (y + dy[k]);
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

func main()
{
  while (cpp_comma(scanf("%d%d", (&h), (&w)), h))
  {
    cpp_statement("rep(i,h) rep(j,w) scanf(\"%d\",B[i]+j); rep(i,h)");
    B[y2][x2] = 7;
    ans = 777;
    dfs(x2, y2, 0);
    printf("%d\n", if ((ans < 777)) ans else 0);
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
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
