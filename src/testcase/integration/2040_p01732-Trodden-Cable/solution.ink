// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var pb: dynamic = cpp_expression("#include<");

var mp: dynamic = cpp_expression("#include<");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func reps(i: dynamic, f: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(f);i<(n);i++)");
}

func each(it: dynamic, v: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((v).begin()) it=(v).begin();it!=(v).end();it++)");
}

func chmin(t: dynamic, f: dynamic) -> dynamic
{
  if ((t > f))
  {
    t = f;
  }
}

func chmax(t: dynamic, f: dynamic) -> dynamic
{
  if ((t < f))
  {
    t = f;
  }
}

var dx: dynamic = [0, 1, 0, -1];

var dy: dynamic = [-1, 0, 1, 0];

var W: dynamic = cpp_uninitialized();

var H: dynamic = cpp_uninitialized();

var N: dynamic = cpp_uninitialized();

var sx: dynamic = cpp_uninitialized();

var sy: dynamic = cpp_uninitialized();

var gx: dynamic = cpp_uninitialized();

var gy: dynamic = cpp_uninitialized();

var cost: dynamic = cpp_array(4, 555, 555);

var dist: dynamic = cpp_array(555, 555);

func add(y: dynamic, x: dynamic, d: dynamic) -> dynamic
{
  if (((((y >= 0) && (y <= H)) && (x >= 0)) && (x <= W)))
  {
    cost[y][x][d] += 1;
  }
}

func conv(c: dynamic) -> dynamic
{
  if ((c == cpp_char("U")))
  {
    return 0;
  }
  if ((c == cpp_char("R")))
  {
    return 1;
  }
  if ((c == cpp_char("D")))
  {
    return 2;
  }
  return 3;
}

func main() -> dynamic
{
  read(W, H, N);
  read(sx, sy, gx, gy);
  var que: dynamic = cpp_uninitialized();
  que.push(node(0, pint(sy, sx)));
  rep(i, (H + 1));
  rep(j, (W + 1))[i][j] = 1001001001001001001;
  dist[sy][sx] = 0;
  while (que.size())
  {
    var y: dynamic = que.top().se.fi;
    var x: dynamic = que.top().se.se;
    var c: dynamic = que.top().fi;
    que.pop();
    if ((dist[y][x] < c))
    {
      continue;
    }
    rep(i, 4);
    {
      var ny: dynamic = (y + dy[i]);
      var nx: dynamic = (x + dx[i]);
      if ((((((ny < 0) || (ny > H)) || (nx < 0)) || (nx > W)) || (dist[ny][nx] <= (c + cost[y][x][i]))))
      {
        continue;
      }
      dist[ny][nx] = (c + cost[y][x][i]);
      que.push(node(dist[ny][nx], pint(ny, nx)));
    }
  }
  write(dist[gy][gx], "\n");
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      rep(i, s.size());
      {
        var d: dynamic = conv(s[i]);
        var ny: dynamic = (y + dy[d]);
        var nx: dynamic = (x + dx[d]);
        if (((((ny < 0) || (ny >= H)) || (nx < 0)) || (nx >= W)))
        {
          continue;
        }
        if ((d == 0))
        {
          add(y, x, 1);
          add(y, (x + 1), 3);
        } else if ((d == 1))
        {
          add(y, (x + 1), 2);
          add((y + 1), (x + 1), 0);
        } else if ((d == 2))
        {
          add((y + 1), x, 1);
          add((y + 1), (x + 1), 3);
        } else
        {
          add(y, x, 2);
          add((y + 1), x, 0);
        }
        y = ny;
        x = nx;
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    var t: dynamic = cpp_uninitialized();
    var s: dynamic = cpp_uninitialized();
    read(x, y, t, s);
  }
