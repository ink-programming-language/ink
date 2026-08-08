// Translated from solution.cpp.

func REP(i: dynamic, b: dynamic, n: dynamic)
{
  cpp_macro("for(int i=b;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<i");
}

var N = cpp_expression("#");

var E = cpp_expression("#");

var S = cpp_expression("#");

var W = cpp_expression("#");

var dx = [0, 1, 0, -1];

var dy = [-1, 0, 1, 0];

var curdir = [S, W, N, E];

var sd = [-1, W, -1, E, S, -1, N, -1];

var cd = [W, -1, -1, N, E, N, -1, -1, -1, S, E, -1, -1, -1, W, S];

var ld = [W, W, -1, E, W, W, -1, N, S, N, N, -1, E, N, N, -1, -1, W, E, E, -1, S, E, E, S, -1, N, S, S, -1, W, S];

var rd = [-1, W, W, E, -1, W, W, S, S, -1, N, N, W, -1, N, N, E, W, -1, E, E, N, -1, E, S, S, N, -1, S, S, E, -1];

var switchid = cpp_array(6, 6);

var rot = cpp_array(6, 6);

var m = cpp_array(6, 6);

var cost = cpp_array(((1 << 6)), 4, 6, 6);

var visited = cpp_array(((1 << 6)), 4, 6, 6);

var ans: dynamic;

func dfs(r: dynamic, c: dynamic, y: dynamic, x: dynamic, d: dynamic, state: dynamic, visnow: dynamic, length: dynamic)
{
  if ((length != 0))
  {
    d = curdir[d];
  }
  if (((((x == -1) || (y == -1)) || (x == c)) || (y == r)))
  {
    return;
  }
  if ((visited[y][x][d][state] == -1))
  {
  } else if ((visited[y][x][d][state] == visnow))
  {
    ans = max(ans, (length - cost[y][x][d][state]));
    return;
  } else
  {
    return;
  }
  visited[y][x][d][state] = visnow;
  cost[y][x][d][state] = length;
  var nextd: dynamic;
  var curgrid = rot[y][x];
  if ((m[y][x] == cpp_char("S")))
  {
    nextd = sd[curgrid][d];
    dfs(r, c, (y + dy[nextd]), (x + dx[nextd]), nextd, state, visnow, (length + 1));
  } else if ((m[y][x] == cpp_char("C")))
  {
    nextd = cd[curgrid][d];
    dfs(r, c, (y + dy[nextd]), (x + dx[nextd]), nextd, state, visnow, (length + 1));
  } else if ((m[y][x] == cpp_char("L")))
  {
    var isswitch = ((((((1 << switchid[y][x])) & state)) != 0));
    nextd = ld[curgrid][isswitch][d];
    if (((((rot[y][x] + 3)) % 4) == d))
    {
      if (isswitch)
      {
        state -= ((1 << switchid[y][x]));
      } else
      {
        state += ((1 << switchid[y][x]));
      }
    }
    dfs(r, c, (y + dy[nextd]), (x + dx[nextd]), nextd, state, visnow, (length + 1));
  } else if ((m[y][x] == cpp_char("R")))
  {
    var isswitch = ((((((1 << switchid[y][x])) & state)) != 0));
    nextd = rd[curgrid][isswitch][d];
    if (((((rot[y][x] + 3)) % 4) == d))
    {
      if (isswitch)
      {
        state -= ((1 << switchid[y][x]));
      } else
      {
        state += ((1 << switchid[y][x]));
      }
    }
    dfs(r, c, (y + dy[nextd]), (x + dx[nextd]), nextd, state, visnow, (length + 1));
  }
}

func solve(r: dynamic, c: dynamic, index: dynamic)
{
  var visnow = 0;
}

func search(r: dynamic, c: dynamic, index: dynamic, now: dynamic, num: dynamic, y: dynamic, x: dynamic, d: dynamic, posx: dynamic, posy: dynamic, tonext: dynamic)
{
  if (((tonext && (num == 0)) && (index == (now + 1))))
  {
    solve(r, c, index);
    return;
  }
  if (tonext)
  {
    if ((num == 0))
    {
      now += 1;
      y = posy[now];
      x = posx[now];
      d = (((rot[y][x] + 3)) % 4);
      search(r, c, index, now, 1, (posy[now] + dy[d]), (posx[now] + dx[d]), d, posx, posy, false);
    } else if ((num == 1))
    {
      y = posy[now];
      x = posx[now];
      d = (((rot[y][x] + 1)) % 4);
      search(r, c, index, now, 2, (posy[now] + dy[d]), (posx[now] + dx[d]), d, posx, posy, false);
    } else if ((num == 2))
    {
      y = posy[now];
      x = posx[now];
      if ((m[y][x] == cpp_char("R")))
      {
        d = (((rot[y][x] + 2)) % 4);
      } else if ((m[posy[now]][posx[now]] == cpp_char("L")))
      {
        d = rot[y][x];
      }
      search(r, c, index, now, 0, (posy[now] + dy[d]), (posx[now] + dx[d]), d, posx, posy, false);
    }
    return;
  }
  if (((((x == -1) || (y == -1)) || (x == c)) || (y == r)))
  {
    return;
  }
  d = curdir[d];
  if ((m[y][x] == cpp_char("S")))
  {
    if ((rot[y][x] == -1))
    {
      if (((d == E) || (d == W)))
      {
        rot[y][x] = 0;
      } else if (((d == S) || (d == N)))
      {
        rot[y][x] = 1;
      }
      search(r, c, index, now, num, (y + dy[sd[rot[y][x]][d]]), (x + dx[sd[rot[y][x]][d]]), sd[rot[y][x]][d], posx, posy, false);
      rot[y][x] = -1;
      return;
    } else if ((rot[y][x] != -1))
    {
      if ((sd[rot[y][x]][d] == -1))
      {
        return;
      } else
      {
        search(r, c, index, now, num, -1, -1, -1, posx, posy, true);
      }
    }
  } else if ((m[y][x] == cpp_char("C")))
  {
    if ((rot[y][x] == -1))
    {
      if ((d == E))
      {
        rot[y][x] = 1;
      } else if ((d == W))
      {
        rot[y][x] = 3;
      } else if ((d == S))
      {
        rot[y][x] = 2;
      } else if ((d == N))
      {
        rot[y][x] = 0;
      }
      search(r, c, index, now, num, (y + dy[cd[rot[y][x]][d]]), (x + dx[cd[rot[y][x]][d]]), cd[rot[y][x]][d], posx, posy, false);
      if ((d == E))
      {
        rot[y][x] = 2;
      } else if ((d == W))
      {
        rot[y][x] = 0;
      } else if ((d == S))
      {
        rot[y][x] = 3;
      } else if ((d == N))
      {
        rot[y][x] = 1;
      }
      search(r, c, index, now, num, (y + dy[cd[rot[y][x]][d]]), (x + dx[cd[rot[y][x]][d]]), cd[rot[y][x]][d], posx, posy, false);
      rot[y][x] = -1;
    } else if ((rot[y][x] != -1))
    {
      if ((cd[rot[y][x]][d] == -1))
      {
        return;
      } else
      {
        search(r, c, index, now, num, -1, -1, -1, posx, posy, true);
      }
    }
  } else if ((m[y][x] == cpp_char("L")))
  {
    if ((ld[rot[y][x]][0][d] == -1))
    {
      return;
    }
    search(r, c, index, now, num, -1, -1, -1, posx, posy, true);
  } else if ((m[y][x] == cpp_char("R")))
  {
    if ((rd[rot[y][x]][0][d] == -1))
    {
      return;
    }
    search(r, c, index, now, num, -1, -1, -1, posx, posy, true);
  }
}

func decide(r: dynamic, c: dynamic, index: dynamic, now: dynamic, posx: dynamic, posy: dynamic)
{
  if ((now == index))
  {
    search(r, c, index, -1, 0, -1, -1, -1, posx, posy, true);
    return;
  }
  rep(i, 4);
  {
    rot[posy[now]][posx[now]] = i;
    decide(r, c, index, (now + 1), posx, posy);
  }
}

func main()
{
  var r: dynamic;
  var c: dynamic;
  while ((((cin >> c) >> r) && c))
  {
    var index = 0;
    var posx = cpp_array(6);
    var posy = cpp_array(6);
    ans = 0;
    decide(r, c, index, 0, posx, posy);
    write(ans, "\n");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      cpp_statement("rep(k,4)");
      {
        rep(l, ((1 << index)));
        {
          visited[i][j][k][l] = -1;
        }
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      if (((m[i][j] == cpp_char("R")) || (m[i][j] == cpp_char("L"))))
      {
        if ((rot[i][j] == 0))
        {
          dfs(r, c, i, j, W, 0, visnow, 0);
        } else if ((rot[i][j] == 1))
        {
          dfs(r, c, i, j, N, 0, visnow, 0);
        } else if ((rot[i][j] == 2))
        {
          dfs(r, c, i, j, E, 0, visnow, 0);
        } else if ((rot[i][j] == 3))
        {
          dfs(r, c, i, j, S, 0, visnow, 0);
        }
        visnow += 1;
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        read(m[i][j]);
        rot[i][j] = -1;
        if (((m[i][j] == cpp_char("R")) || (m[i][j] == cpp_char("L"))))
        {
          switchid[i][j] = index;
          posy[index] = i;
          posx[index] = j;
          index += 1;
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    }
