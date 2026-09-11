// Translated from solution.cpp.

func Q_Q() -> dynamic
{
  ios.sync_with_stdio(0);
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

func main() -> dynamic
{
  Q_Q();
  var n: dynamic = cpp_uninitialized();
  read(n);
  var arr: dynamic = cpp_array(n);
  var dq: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  var a: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = (n - 1);
    while ((i > 0))
    {
      if ((i == (n - 1)))
      {
        if ((arr[i] <= arr[0]))
        {
          dq.push_front(arr[i]);
          ans += 1;
        } else
        {
          break;
        }
      } else
      {
        if ((arr[i] <= arr[(i + 1)]))
        {
          dq.push_front(arr[i]);
          ans += 1;
        } else
        {
          break;
        }
      }
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      dq.push_back(arr[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((i != (n - 1)))
      {
        if ((dq[i] > dq[(i + 1)]))
        {
          write(-1);
          return 0;
        } else if ((dq[i] == dq[(i + 1)]))
        {
          a += 1;
        }
      }
      i += 1;
    }
  }
  if ((a == n))
  {
    write(0);
  } else
  {
    write(ans);
  }
  return 0;
}
