// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    n *= 2;
    var a: dynamic = cpp_array(n);
    var ve: dynamic = cpp_uninitialized();
    var vo: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        if (((a[i] % 2) == 0))
        {
          ve.push_back((i + 1));
        } else
        {
          vo.push_back((i + 1));
        }
        i += 1;
      }
    }
    if (((vo.size() % 2) == 0))
    {
      if ((vo.size() == 0))
      {
        {
          var i: dynamic = 2;
          while ((i < ve.size()))
          {
            write(ve[i], " ", ve[(i + 1)], "\n");
            i += 2;
          }
        }
      } else if ((ve.size() == 0))
      {
        {
          var i: dynamic = 2;
          while ((i < vo.size()))
          {
            write(vo[i], " ", vo[(i + 1)], "\n");
            i += 2;
          }
        }
      } else
      {
        {
          var i: dynamic = 0;
          while ((i < vo.size()))
          {
            write(vo[i], " ", vo[(i + 1)], "\n");
            i += 2;
          }
        }
        {
          var i: dynamic = 2;
          while ((i < ve.size()))
          {
            write(ve[i], " ", ve[(i + 1)], "\n");
            i += 2;
          }
        }
      }
    } else
    {
      {
        var i: dynamic = 1;
        while ((i < vo.size()))
        {
          write(vo[i], " ", vo[(i + 1)], "\n");
          i += 2;
        }
      }
      {
        var i: dynamic = 1;
        while ((i < ve.size()))
        {
          write(ve[i], " ", ve[(i + 1)], "\n");
          i += 2;
        }
      }
    }
  }
}
