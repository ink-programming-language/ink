// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      {
        var j: dynamic = 0;
        while ((j < N))
        {
          dist[i][j] = min([i, ((N - 1) - i), j, ((N - 1) - j)]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (N * N)))
    {
      var p: dynamic = cpp_uninitialized();
      read(p);
      p -= 1;
      var h: dynamic = (p / N);
      var w: dynamic = (p % N);
      ans += dist[h][w];
      exist[h][w] = false;
      var Q: dynamic = cpp_uninitialized();
      Q.emplace(h, w);
      var dx: dynamic = [1, 0, -1, 0];
      var dy: dynamic = [0, 1, 0, -1];
      while (Q.size())
      {
        var (x, y): dynamic = Q.front();
        Q.pop();
        var d: dynamic = (dist[x][y] + exist[x][y]);
        {
          var i: dynamic = 0;
          while ((i < 4))
          {
            var x: dynamic = (x + dx[i]);
            var y: dynamic = (y + dy[i]);
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
