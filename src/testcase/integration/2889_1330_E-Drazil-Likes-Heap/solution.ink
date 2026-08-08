// Translated from solution.cpp.

var a = cpp_array((((1 << 21)) + 1));

func wid(i: dynamic)
{
  if (((a[(2 * i)] == 0) && (a[((2 * i) + 1)] == 0)))
  {
    return i;
  } else
  {
    if ((a[(2 * i)] > a[((2 * i) + 1)]))
    {
      return wid((2 * i));
    } else
    {
      return wid(((2 * i) + 1));
    }
  }
}

func f(i: dynamic)
{
  if (((a[(2 * i)] == 0) && (a[((2 * i) + 1)] == 0)))
  {
    a[i] = 0;
  } else
  {
    if ((a[(2 * i)] > a[((2 * i) + 1)]))
    {
      a[i] = a[(2 * i)];
      f((2 * i));
    } else
    {
      a[i] = a[((2 * i) + 1)];
      f(((2 * i) + 1));
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var h: dynamic;
  var g: dynamic;
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(h, g);
    var n = ((1 << h));
    var x: dynamic;
    {
      var i = 1;
      while ((i < ((1 << ((h + 1))))))
      {
        if ((i < n))
        {
          read(a[i]);
        } else
        {
          a[i] = 0;
        }
        i += 1;
      }
    }
    var index = (((1 << g)) - 1);
    {
      var i = 1;
      while ((i < ((1 << g))))
      {
        var t = wid(i);
        while ((t > index))
        {
          x.push_back(i);
          f(i);
          if ((x.size() == (((1 << h)) - ((1 << g)))))
          {
            break;
          }
          t = wid(i);
        }
        if ((x.size() == (((1 << h)) - ((1 << g)))))
        {
          break;
        }
        i += 1;
      }
    }
    var sum = 0;
    {
      var i = 1;
      while ((i < ((1 << g))))
      {
        sum += a[i];
        i += 1;
      }
    }
    write(sum, cpp_char("\n"));
    {
      var i = 0;
      while ((i < x.size()))
      {
        write(x[i], " ");
        i += 1;
      }
    }
    write(cpp_char("\n"));
  }
  return 0;
}
