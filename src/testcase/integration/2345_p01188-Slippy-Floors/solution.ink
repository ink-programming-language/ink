// Translated from solution.cpp.

func REP(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for(int i=s;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("/* 00:42 -");
}

var IINF = cpp_expression("/* 00:42");

var MAX = cpp_expression("/*");

var H: dynamic;

var W: dynamic;

var used = cpp_array(MAX, MAX);

var G = cpp_array(MAX, MAX);

var sp: dynamic;

var mincost: dynamic;

var dx = [0, 1, 0, -1];

var dy = [1, 0, -1, 0];

func isValid(x: dynamic, y: dynamic)
{
  return (((((0 <= x) && (x < W)) && (0 <= y)) && (y < H)));
}

func dfs(cur: dynamic, cost: dynamic, pdir: dynamic)
{
  if ((cost >= 10))
  {
    return;
  }
  if ((mincost <= cost))
  {
    return;
  }
  var x = (cur % W);
  var y = (cur / W);
  if ((G[y][x] == cpp_char(">")))
  {
    mincost = min(mincost, cost);
    return;
  }
  if (used[y][x])
  {
    return;
  }
  rep(i, 4);
  {
    if ((i == pdir))
    {
      continue;
    }
    var rnx = (x + dx[(((i + 2)) % 4)]);
    var rny = (y + dy[(((i + 2)) % 4)]);
    if ((G[rny][rnx] == cpp_char("#")))
    {
      used[y][x] = true;
      var nx = (x + dx[i]);
      var ny = (y + dy[i]);
      while ((G[ny][nx] == cpp_char("_")))
      {
        if ((G[(ny + dy[i])][(nx + dx[i])] == cpp_char("^")))
        {
          break;
        }
        if ((G[(ny + dy[i])][(nx + dx[i])] == cpp_char("_")))
        {
          G[(ny + dy[i])][(nx + dx[i])] = cpp_char("#");
          dfs((nx + (ny * W)), (cost + 1), (((i + 2)) % 4));
          G[(ny + dy[i])][(nx + dx[i])] = cpp_char("_");
        }
        nx += dx[i];
        ny += dy[i];
      }
      if ((G[ny][nx] == cpp_char(">")))
      {
        dfs((nx + (ny * W)), cost, (((i + 2)) % 4));
      }
      if ((G[ny][nx] == cpp_char("#")))
      {
        nx -= dx[i];
        ny -= dy[i];
        dfs((nx + (ny * W)), cost, (((i + 2)) % 4));
      }
      used[y][x] = false;
    } else if ((((rnx + (rny * W)) != sp) && (G[rny][rnx] == cpp_char("_"))))
    {
      G[rny][rnx] = cpp_char("#");
      dfs(cur, (cost + 1), pdir);
      G[rny][rnx] = cpp_char("_");
    }
  }
}

func main()
{
  var T: dynamic;
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    scanf("%d %d", (&H), (&W));
    rep(y, H);
    mincost = IINF;
    dfs(sp, 0, IINF);
    if ((mincost == IINF))
    {
      puts("10");
    } else
    {
      printf("%d\n", mincost);
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      read(G[y][x]);
      if ((G[y][x] == cpp_char("A")))
      {
        sp = (x + (y * W));
        G[y][x] = cpp_char("_");
      }
      used[y][x] = false;
    }
