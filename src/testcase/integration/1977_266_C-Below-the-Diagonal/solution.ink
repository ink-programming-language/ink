// Translated from solution.cpp.

var MAX = (1e3 + 5);

var grid = cpp_array(MAX, MAX);

var row_sum = cpp_array(MAX);

var col_sum = cpp_array(MAX);

var ans: dynamic;

func swap_row(x: dynamic, y: dynamic)
{
  ans.emplace_back(1, x, y);
  swap(grid[x], grid[y]);
  swap(row_sum[x], row_sum[y]);
}

func swap_col(x: dynamic, y: dynamic, n: dynamic)
{
  ans.emplace_back(2, x, y);
  {
    var i = int_cpp(1);
    while ((i < int_cpp((n + 1))))
    {
      swap(grid[i][x], grid[i][y]);
      i += 1;
    }
  }
  swap(col_sum[x], col_sum[y]);
}

func roll(n: dynamic)
{
  var zero_col = -1;
  {
    var i = int_cpp(1);
    while ((i < int_cpp((n + 1))))
    {
      if ((col_sum[i] == 0))
      {
        zero_col = i;
      }
      i += 1;
    }
  }
  if ((zero_col != n))
  {
    swap_col(zero_col, n, n);
  }
  var one_row = -1;
  {
    var i = int_cpp(1);
    while ((i < int_cpp((n + 1))))
    {
      if ((row_sum[i] > 0))
      {
        one_row = i;
      }
      i += 1;
    }
  }
  if ((one_row > -1))
  {
    if ((one_row != n))
    {
      swap_row(one_row, n);
    }
    {
      var i = int_cpp(1);
      while ((i < int_cpp((n + 1))))
      {
        col_sum[i] -= grid[n][i];
        row_sum[i] -= grid[i][n];
        i += 1;
      }
    }
    roll((n - 1));
  }
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = int_cpp(0);
    while ((i < int_cpp((n - 1))))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d %d", (&x), (&y));
      grid[x][y] = 1;
      row_sum[x] += 1;
      col_sum[y] += 1;
      i += 1;
    }
  }
  roll(n);
  printf("%d\n", cpp_cast(ans.size()));
  for (var each in ans)
  {
    var x: dynamic;
    var y: dynamic;
    var z: dynamic;
    tie(x, y, z) = each;
    printf("%d %d %d\n", x, y, z);
  }
  return 0;
}
