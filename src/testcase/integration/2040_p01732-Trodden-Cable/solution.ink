// Translated from solution.cpp.

var int_cpp = dynamic;

var pb = cpp_expression("#include<");

var mp = cpp_expression("#include<");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func all(v: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func reps(i: dynamic, f: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(f);i<(n);i++)");
}

func each(it: dynamic, v: dynamic)
{
  cpp_macro("for(__typeof((v).begin()) it=(v).begin();it!=(v).end();it++)");
}

func chmin(t: dynamic, f: dynamic)
{
  if ((t > f))
  {
    t = f;
  }
}

func chmax(t: dynamic, f: dynamic)
{
  if ((t < f))
  {
    t = f;
  }
}

var dx = [0, 1, 0, -1];

var dy = [-1, 0, 1, 0];

var W: dynamic;

var H: dynamic;

var N: dynamic;

var sx: dynamic;

var sy: dynamic;

var gx: dynamic;

var gy: dynamic;

var cost = cpp_array(4, 555, 555);

var dist = cpp_array(555, 555);

func add(y: dynamic, x: dynamic, d: dynamic)
{
  if (((((y >= 0) && (y <= H)) && (x >= 0)) && (x <= W)))
  {
    cost[y][x][d] += 1;
  }
}

func conv(c: dynamic)
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

func main()
{
  read(W, H, N);
  read(sx, sy, gx, gy);
  var que: dynamic;
  que.push(node(0, pint(sy, sx)));
  rep(i, (H + 1));
  rep(j, (W + 1))[i][j] = 1001001001001001001;
  dist[sy][sx] = 0;
  while (que.size())
  {
    var y = que.top().se.fi;
    var x = que.top().se.se;
    var c = que.top().fi;
    que.pop();
    if ((dist[y][x] < c))
    {
      continue;
    }
    rep(i, 4);
    {
      var ny = (y + dy[i]);
      var nx = (x + dx[i]);
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
      rep(i, s.size());
      {
        var d = conv(s[i]);
        var ny = (y + dy[d]);
        var nx = (x + dx[d]);
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var x: dynamic;
    var y: dynamic;
    var t: dynamic;
    var s: dynamic;
    read(x, y, t, s);
  }
