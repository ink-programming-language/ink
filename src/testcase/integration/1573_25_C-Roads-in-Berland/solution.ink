// Translated from solution.cpp.

var grid = cpp_array(305, 305);

var result = cpp_array(305, 305);

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  var i: dynamic;
  var j: dynamic;
  var t: dynamic;
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var l: dynamic;
  var r: dynamic;
  var temp: dynamic;
  var sum: dynamic;
  var mini: dynamic;
  var maxi: dynamic;
  var flag: dynamic;
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
