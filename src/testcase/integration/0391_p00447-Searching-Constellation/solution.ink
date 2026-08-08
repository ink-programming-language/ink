// Translated from solution.cpp.

func f(a: dynamic, b: dynamic)
{
  return make_pair((b.first - a.first), (b.second - a.second));
}

func main()
{
  var m: dynamic;
  var n: dynamic;
  while (true)
  {
    read(m);
    if ((m == 0))
    {
      break;
    }
    {
      var i = 0;
      while ((i < m))
      {
        read(seiza[i].first, seiza[i].second);
        i += 1;
      }
    }
    read(n);
    {
      var i = 0;
      while ((i < n))
      {
        read(image[i].first, image[i].second);
        i += 1;
      }
    }
    var s0 = seiza[0];
    {
      var i = 0;
      while ((i < n))
      {
        var cnt = 1;
        var amount_of_change = f(s0, image[i]);
        {
          var j = 1;
          while ((j < m))
          {
            var sj = seiza[j];
            {
              var k = 0;
              while ((k < n))
              {
                if ((((sj.first + amount_of_change.first) == image[k].first) && ((sj.second + amount_of_change.second) == image[k].second)))
                {
                  cnt += 1;
                }
                k += 1;
              }
            }
            if ((cnt == m))
            {
              printf("%d %d\n", amount_of_change.first, amount_of_change.second);
              cpp_goto("goto end;");
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
}
