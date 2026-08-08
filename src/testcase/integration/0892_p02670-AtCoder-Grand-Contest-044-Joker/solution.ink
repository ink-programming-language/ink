// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = 0;
        while ((j < N))
        {
          dist[i][j] = min([i, ((N - 1) - i), j, ((N - 1) - j)]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < (N * N)))
    {
      var p: dynamic;
      read(p);
      p -= 1;
      var h = (p / N);
      var w = (p % N);
      ans += dist[h][w];
      exist[h][w] = false;
      var Q: dynamic;
      Q.emplace(h, w);
      var dx = [1, 0, -1, 0];
      var dy = [0, 1, 0, -1];
      while (Q.size())
      {
        var (x, y) = Q.front();
        Q.pop();
        var d = (dist[x][y] + exist[x][y]);
        {
          var i = 0;
          while ((i < 4))
          {
            var x = (x + dx[i]);
            var y = (y + dy[i]);
            if (cpp_binary(cpp_binary(cpp_binary(cpp_binary((x >= 0), "and", (x < N)), "and", (y >= 0)), "and", (y < N)), "and", (dist[x][y] > d)))
            {
              dist[x][y] = d;
              Q.emplace(x, y);
            }
            i += 1;
          }
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
