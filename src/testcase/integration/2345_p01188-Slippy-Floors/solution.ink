// Translated from solution.cpp.

func REP(i: dynamic, s: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=s;i<n;i++)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("/* 00:42 -");
}

var IINF: dynamic = cpp_expression("/* 00:42");

var MAX: dynamic = cpp_expression("/*");

var H: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var used: dynamic = cpp_array(MAX, MAX);

var G: dynamic = cpp_array(MAX, MAX);

var sp: dynamic = cpp_uninitialized();

var mincost: dynamic = cpp_uninitialized();

var dx: dynamic = [0, 1, 0, -1];

var dy: dynamic = [1, 0, -1, 0];

func isValid(x: dynamic, y: dynamic) -> dynamic
{
  return (((((0 <= x) && (x < W)) && (0 <= y)) && (y < H)));
}

func dfs(cur: dynamic, cost: dynamic, pdir: dynamic) -> dynamic
{
  if ((cost >= 10))
  {
    return;
  }
  if ((mincost <= cost))
  {
    return;
  }
  var x: dynamic = (cur % W);
  var y: dynamic = (cur / W);
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
    var rnx: dynamic = (x + dx[(((i + 2)) % 4)]);
    var rny: dynamic = (y + dy[(((i + 2)) % 4)]);
    if ((G[rny][rnx] == cpp_char("#")))
    {
      used[y][x] = true;
      var nx: dynamic = (x + dx[i]);
      var ny: dynamic = (y + dy[i]);
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

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      read(G[y][x]);
      if ((G[y][x] == cpp_char("A")))
      {
        sp = (x + (y * W));
        G[y][x] = cpp_char("_");
      }
      used[y][x] = false;
    }
