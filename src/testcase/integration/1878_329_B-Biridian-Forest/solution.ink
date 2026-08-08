// Translated from solution.cpp.

var N = 1005;

var INF = 1e9;

var D = cpp_array(N, N);

var n: dynamic;

var m: dynamic;

var er: dynamic;

var ec: dynamic;

var sr: dynamic;

var sc: dynamic;

var k: dynamic;

var mark = cpp_array(N, N);

var t = cpp_array(N, N);

var dx = [0, -1, 1, 0];

var dy = [-1, 0, 0, 1];

var q: dynamic;

func bfs(r: dynamic, c: dynamic)
{
  mark[r][c] = true;
  D[r][c] = 0;
  q.push([r, c]);
  while (q.size())
  {
    var v = q.front();
    q.pop();
    {
      var i = 0;
      while ((i < 4))
      {
        var u = [(v.first + dx[i]), (v.second + dy[i])];
        if (((((((!mark[u.first][u.second]) && (t[u.first][u.second] != cpp_char("T"))) && (u.first <= n)) && (u.first > 0)) && (u.second <= m)) && (u.second > 0)))
        {
          mark[u.first][u.second] = true;
          D[u.first][u.second] = (D[v.first][v.second] + 1);
          q.push(u);
        }
        i += 1;
      }
    }
  }
}

func main()
{
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          read(t[i][j]);
          if ((t[i][j] == cpp_char("E")))
          {
            er = i;
            ec = j;
          }
          if ((t[i][j] == cpp_char("S")))
          {
            sr = i;
            sc = j;
          }
          D[i][j] = INF;
          j += 1;
        }
      }
      i += 1;
    }
  }
  bfs(er, ec);
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          if (((D[i][j] <= D[sr][sc]) && isdigit(t[i][j])))
          {
            k += (t[i][j] - cpp_char("0"));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(k);
}
