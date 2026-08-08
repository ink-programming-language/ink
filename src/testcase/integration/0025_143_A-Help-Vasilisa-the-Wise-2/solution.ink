// Translated from solution.cpp.

func main()
{
  var l = 0;
  var b = 0;
  var v = cpp_construct(3);
  {
    var i = 0;
    while ((i < 3))
    {
      {
        var j = 0;
        while ((j < 2))
        {
          read(l);
          v[i].push_back(l);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var x = ((((v[2][1] - v[0][1]) + v[1][1])) / 2);
  var a = cpp_array(4);
  a[0] = (v[0][0] - x);
  a[1] = x;
  a[2] = ((v[0][1] - v[1][1]) + x);
  a[3] = (v[1][1] - x);
  {
    var i = 0;
    while ((i < 4))
    {
      {
        var j = (i + 1);
        while ((j < 4))
        {
          if ((a[i] == a[j]))
          {
            b = 1;
            j = 4;
            i = 4;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 4))
    {
      if (((a[i] < 1) || (a[i] > 9)))
      {
        b = 1;
        i = 4;
      }
      i += 1;
    }
  }
  if (((a[2] + a[0]) != v[1][0]))
  {
    b = 1;
  }
  if (b)
  {
    write(-1, "\n");
  } else
  {
    write(a[0], " ", a[1], "\n");
    write(a[2], " ", a[3], "\n");
  }
  return 0;
}
