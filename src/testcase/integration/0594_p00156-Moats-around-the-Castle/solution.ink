// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func ALL(A: dynamic)
{
  return cpp_expression("#include <iostream");
}

var INF = 100000000;

var MAX_N = 100;

var MAX_M = 100;

var dy = [-1, 0, 1, 0];

var dx = [0, 1, 0, -1];

var maze = cpp_array((MAX_M + 1), MAX_N);

var N: dynamic;

var M: dynamic;

var sy: dynamic;

var sx: dynamic;

var cost = cpp_array(MAX_M, MAX_N);

func bfs(argument_0: dynamic)
{
  rep(i, N)(j, M);
  cost[i][j] = INF;
  var que: dynamic;
  que.push(P(sy, sx));
  cost[sy][sx] = 0;
  while ((!que.empty()))
  {
    var cur = que.front();
    que.pop();
    var cy = cur.first;
    var cx = cur.second;
    var cc = cost[cy][cx];
    rep(k, 4);
    {
      var ny = (cy + dy[k]);
      var nx = (cx + dx[k]);
      if (((((ny < 0) || (ny >= N)) || (nx < 0)) || (nx >= M)))
      {
        continue;
      }
      var next_cost = (cost[cy][cx] + (if ((((maze[ny][nx] == cpp_char("#")) && (maze[cy][cx] != cpp_char("#"))))) 1 else 0));
      if ((next_cost >= cost[ny][nx]))
      {
        continue;
      }
      cost[ny][nx] = next_cost;
      que.push(P(ny, nx));
    }
  }
  var res = INF;
  rep(i, N) = min(res, cost[i][0]);
  rep(i, N) = min(res, cost[i][(M - 1)]);
  rep(j, M) = min(res, cost[0][j]);
  rep(j, M) = min(res, cost[(N - 1)][j]);
  return res;
}

func main()
{
  ios_base.sync_with_stdio(0);
  while (cpp_comma(((cin >> M) >> N), M))
  {
    memset(maze, 0, cpp_sizeof((maze)));
    memset(cost, 0, cpp_sizeof((cost)));
    rep(i, N)(j, M);
    read(maze[i][j]);
    rep(i, N)(j, M);
    if ((maze[i][j] == cpp_char("&")))
    {
      sy = i;
      sx = j;
    }
    var res = bfs();
    write(res, "\n");
  }
  return 0;
}
