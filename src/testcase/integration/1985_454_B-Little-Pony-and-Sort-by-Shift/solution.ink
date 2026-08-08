// Translated from solution.cpp.

func Q_Q()
{
  ios.sync_with_stdio(0);
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

func main()
{
  Q_Q();
  var n: dynamic;
  read(n);
  var arr = cpp_array(n);
  var dq: dynamic;
  var ans = 0;
  var a = 1;
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  {
    var i = (n - 1);
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
    var i = 0;
    while ((i < n))
    {
      dq.push_back(arr[i]);
      i += 1;
    }
  }
  {
    var i = 0;
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
