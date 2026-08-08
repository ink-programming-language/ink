// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    n *= 2;
    var a = cpp_array(n);
    var ve: dynamic;
    var vo: dynamic;
    {
      var i = 0;
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
          var i = 2;
          while ((i < ve.size()))
          {
            write(ve[i], " ", ve[(i + 1)], "\n");
            i += 2;
          }
        }
      } else if ((ve.size() == 0))
      {
        {
          var i = 2;
          while ((i < vo.size()))
          {
            write(vo[i], " ", vo[(i + 1)], "\n");
            i += 2;
          }
        }
      } else
      {
        {
          var i = 0;
          while ((i < vo.size()))
          {
            write(vo[i], " ", vo[(i + 1)], "\n");
            i += 2;
          }
        }
        {
          var i = 2;
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
        var i = 1;
        while ((i < vo.size()))
        {
          write(vo[i], " ", vo[(i + 1)], "\n");
          i += 2;
        }
      }
      {
        var i = 1;
        while ((i < ve.size()))
        {
          write(ve[i], " ", ve[(i + 1)], "\n");
          i += 2;
        }
      }
    }
  }
}
