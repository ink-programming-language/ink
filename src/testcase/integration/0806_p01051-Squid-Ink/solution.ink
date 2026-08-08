// Translated from solution.cpp.

var MAX_R = cpp_expression("#incl");

var MAX_C = cpp_expression("#i");

var INF = cpp_expression("#inclu");

var field = cpp_array((MAX_C + 1), MAX_R);

var used = cpp_array((MAX_C + 1), MAX_R);

var d = cpp_array((MAX_C + 1), MAX_R);

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

var R: dynamic;

var C: dynamic;

var start: dynamic;

var goal: dynamic;

func dijkstra(s: dynamic)
{
  d[s.second][s.first] = 0;
  while (1)
  {
    var v = cpp_construct(-1, -1);
    {
      var i = 0;
      while ((i < R))
      {
        {
          var j = 0;
          while ((j < C))
          {
            if (((!used[i][j]) && (((v == coordinate(-1, -1)) || (d[i][j] < d[v.second][v.first])))))
            {
              v = coordinate(j, i);
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    if ((v == coordinate(-1, -1)))
    {
      break;
    }
    used[v.second][v.first] = true;
    {
      var i = 0;
      while ((i <= 4))
      {
        var nx = (v.first + dx[i]);
        var ny = (v.second + dy[i]);
        if ((((((0 <= nx) && (nx < C)) && (0 <= ny)) && (ny < R)) && (field[ny][nx] != cpp_char("#"))))
        {
          if ((field[ny][nx] == cpp_char("o")))
          {
            d[ny][nx] = min(d[ny][nx], (d[v.second][v.first] + 1));
          } else if ((field[ny][nx] == cpp_char("x")))
          {
            d[ny][nx] = min(d[ny][nx], (d[v.second][v.first] + 3));
          } else
          {
            d[ny][nx] = min(d[ny][nx], (d[v.second][v.first] + 2));
          }
          var nnx = (nx + dx[i]);
          var nny = (ny + dy[i]);
          if ((((((0 <= nnx) && (nnx < C)) && (0 <= nny)) && (nny < R)) && (field[nny][nnx] != cpp_char("#"))))
          {
            d[nny][nnx] = min(d[nny][nnx], (d[v.second][v.first] + 4));
            var nnnx = (nnx + dx[i]);
            var nnny = (nny + dy[i]);
            if ((((((0 <= nnnx) && (nnnx < C)) && (0 <= nnny)) && (nnny < R)) && (field[nnny][nnnx] != cpp_char("#"))))
            {
              d[nnny][nnnx] = min(d[nnny][nnnx], (d[v.second][v.first] + 5));
            }
          }
        }
        i += 1;
      }
    }
  }
}

func main()
{
  var tmp: dynamic;
  read(R, C);
  {
    var i = 0;
    while ((i < R))
    {
      fill(d[i], (d[i] + C), INF);
      fill(used[i], (used[i] + C), false);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < R))
    {
      {
        var j = 0;
        while ((j < C))
        {
          read(tmp);
          if ((tmp == cpp_char("S")))
          {
            start = coordinate(j, i);
          }
          if ((tmp == cpp_char("G")))
          {
            goal = coordinate(j, i);
          }
          if ((tmp == cpp_char("#")))
          {
            used[i][j] = true;
          }
          field[i][j] = tmp;
          j += 1;
        }
      }
      i += 1;
    }
  }
  dijkstra(start);
  write(d[goal.second][goal.first], "\n");
  return 0;
}
