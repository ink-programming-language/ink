// Translated from solution.cpp.

var is_square: dynamic = cpp_uninitialized();

func generate_squares(argument_0: dynamic) -> dynamic
{
  is_square.push_back(0);
  is_square.push_back(1);
  {
    var i: dynamic = 2;
    while (((i * i) <= int_cpp(1e10)))
    {
      is_square.push_back((i * i));
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  for (var e: dynamic in v)
  {
    read(e);
  }
  generate_squares();
  var squares: dynamic = cpp_uninitialized();
  var not_squares: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var idx: dynamic = (lower_bound(is_square.begin(), is_square.end(), v[i]) - is_square.begin());
      if ((is_square[idx] == v[i]))
      {
        squares.push_back(v[i]);
      } else
      {
        not_squares.push_back(v[i]);
        cost[v[i]] = min((v[i] - is_square[(idx - 1)]), (is_square[idx] - v[i]));
      }
      i += 1;
    }
  }
  sort(squares.begin(), squares.end());
  sort(not_squares.begin(), not_squares.end(), __cpp_lambda_1);
  var cnt_squares: dynamic = cpp_cast(squares.size());
  var cnt_notsquares: dynamic = cpp_cast(not_squares.size());
  var ans: dynamic = 0;
  if ((cnt_squares == cnt_notsquares))
  {
    write(0);
    return 0;
  }
  if ((cnt_squares > cnt_notsquares))
  {
    {
      var i: dynamic = (cnt_squares - 1);
      while ((i >= 0))
      {
        if ((squares[i] > 0))
        {
          ans += 1;
        } else
        {
          ans += 2;
        }
        cnt_notsquares += 1;
        if ((cnt_notsquares == (n / 2)))
        {
          break;
        }
        i -= 1;
      }
    }
  } else
  {
    {
      var i: dynamic = 0;
      while ((i < cnt_notsquares))
      {
        ans += cost[not_squares[i]];
        cnt_squares += 1;
        if ((cnt_squares == (n / 2)))
        {
          break;
        }
        i += 1;
      }
    }
  }
  write(ans);
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return (cost[a] < cost[b]);
}
