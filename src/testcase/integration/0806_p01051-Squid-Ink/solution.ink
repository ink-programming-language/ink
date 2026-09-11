// Translated from solution.cpp.

var MAX_R: dynamic = cpp_expression("#incl");

var MAX_C: dynamic = cpp_expression("#i");

var INF: dynamic = cpp_expression("#inclu");

var field: dynamic = cpp_array((MAX_C + 1), MAX_R);

var used: dynamic = cpp_array((MAX_C + 1), MAX_R);

var d: dynamic = cpp_array((MAX_C + 1), MAX_R);

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

var R: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

var start: dynamic = cpp_uninitialized();

var goal: dynamic = cpp_uninitialized();

func dijkstra(s: dynamic) -> dynamic
{
  d[s.second][s.first] = 0;
  while (1)
  {
    var v: dynamic = cpp_construct(-1, -1);
    {
      var i: dynamic = 0;
      while ((i < R))
      {
        {
          var j: dynamic = 0;
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
      var i: dynamic = 0;
      while ((i <= 4))
      {
        var nx: dynamic = (v.first + dx[i]);
        var ny: dynamic = (v.second + dy[i]);
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
          var nnx: dynamic = (nx + dx[i]);
          var nny: dynamic = (ny + dy[i]);
          if ((((((0 <= nnx) && (nnx < C)) && (0 <= nny)) && (nny < R)) && (field[nny][nnx] != cpp_char("#"))))
          {
            d[nny][nnx] = min(d[nny][nnx], (d[v.second][v.first] + 4));
            var nnnx: dynamic = (nnx + dx[i]);
            var nnny: dynamic = (nny + dy[i]);
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

func main() -> dynamic
{
  var tmp: dynamic = cpp_uninitialized();
  read(R, C);
  {
    var i: dynamic = 0;
    while ((i < R))
    {
      fill(d[i], (d[i] + C), INF);
      fill(used[i], (used[i] + C), false);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < R))
    {
      {
        var j: dynamic = 0;
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
