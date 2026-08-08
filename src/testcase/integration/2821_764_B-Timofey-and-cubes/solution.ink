// Translated from solution.cpp.

func isodd(x: dynamic)
{
  return (x % 2);
}

func main()
{
  var n: dynamic;
  read(n);
  var ans: dynamic;
  {
    var i = (n - 1);
    while ((i != -1))
    {
      read(arr[i]);
      i -= 1;
    }
  }
  if ((n == 1))
  {
    write(arr[0]);
    return 0;
  }
  ans.push_back(arr[0]);
  var size = ((n / 2) - 1);
  var p = cpp_array(size);
  var b = 1;
  var e = (n - 2);
  {
    var i = 0;
    while ((i < size))
    {
      p[i] = make_pair(arr[e], arr[b]);
      b += 1;
      e -= 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < size))
    {
      if ((i % 2))
      {
        ans.push_back(p[i].second);
      } else
      {
        ans.push_back(p[i].first);
      }
      i += 1;
    }
  }
  if (((n % 2) == 1))
  {
    ans.push_back(arr[(n / 2)]);
  }
  {
    var i = (size - 1);
    while ((i != -1))
    {
      if ((i % 2))
      {
        ans.push_back(p[i].first);
      } else
      {
        ans.push_back(p[i].second);
      }
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  write(arr[(n - 1)]);
}
