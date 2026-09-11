// Translated from solution.cpp.

var grid: dynamic = cpp_array(305, 305);

var result: dynamic = cpp_array(305, 305);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
  var mini: dynamic = cpp_uninitialized();
  var maxi: dynamic = cpp_uninitialized();
  var flag: dynamic = cpp_uninitialized();
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= n))
        {
          read(grid[i][j]);
          result[i][j] = grid[i][j];
          j += 1;
        }
      }
      i += 1;
    }
  }
  read(k);
  while (cpp_update(k, "--"))
  {
    sum = 0;
    read(l, r);
    read(m);
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = 1;
          while ((j <= n))
          {
            result[i][j] = min(grid[i][j], min(((grid[i][l] + m) + result[r][j]), ((grid[i][r] + m) + grid[l][j])));
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = 1;
          while ((j <= n))
          {
            grid[i][j] = result[i][j];
            sum += result[i][j];
            j += 1;
          }
        }
        i += 1;
      }
    }
    write((sum / 2), " ");
  }
  write("\n");
  return 0;
}
