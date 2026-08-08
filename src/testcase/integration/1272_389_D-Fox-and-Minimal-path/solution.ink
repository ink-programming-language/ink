// Translated from solution.cpp.

func powInt(x: dynamic, y: dynamic)
{
  var r = 1;
  while ((y > 0))
  {
    if ((y & 1))
    {
      r = ((r * x));
    }
    x = ((x * x));
    y /= 2;
  }
  return r;
}

func qrt(k: dynamic, ex: dynamic)
{
  var res = 1;
  while ((powInt(res, ex) <= k))
  {
    res += 1;
  }
  return (res - 1);
}

func main()
{
  var k: dynamic;
  read(k);
  var nds: dynamic;
  var ex = 3;
  var sum: dynamic;
  var n: dynamic;
  while (true)
  {
    var tmpk = k;
    sum = 0;
    while ((tmpk > 0))
    {
      var tmp = qrt(cpp_cast(tmpk), ex);
      nds.push_back(tmp);
      tmpk -= powInt(tmp, ex);
    }
    {
      int_cpp((i)) = (0);
      while ((((i)) < ((nds.size()))))
      {
        sum += nds[i];
        ((i)) += 1;
      }
    }
    n = ((ex * sum) + 2);
    if ((n > 1000))
    {
      nds.clear();
      ex += 1;
    }
    if (!(((n > 1000))))
    {
      break;
    }
  }
  var init = cpp_construct(n, cpp_char("N"));
  {
    int_cpp(i) = (2);
    while (((i) < ((sum + 2))))
    {
      g[0][i] = cpp_char("Y");
      g[i][0] = cpp_char("Y");
      g[1][(i + (((ex - 1)) * sum))] = cpp_char("Y");
      g[(i + (((ex - 1)) * sum))][1] = cpp_char("Y");
      (i) += 1;
    }
  }
  var offset = 2;
  write(n, "\n");
  {
    int_cpp((i)) = (0);
    while ((((i)) < ((nds.size()))))
    {
      var c = nds[i];
      {
        int_cpp(j) = (offset);
        while (((j) < ((offset + c))))
        {
          {
            int_cpp(q) = (0);
            while (((q) < ((ex - 1))))
            {
              {
                int_cpp(p) = (0);
                while (((p) < (nds[i])))
                {
                  g[(j + (q * sum))][(((((q + 1)) * sum) + p) + offset)] = cpp_char("Y");
                  g[(((((q + 1)) * sum) + p) + offset)][(j + (q * sum))] = cpp_char("Y");
                  (p) += 1;
                }
              }
              (q) += 1;
            }
          }
          (j) += 1;
        }
      }
      offset += c;
      ((i)) += 1;
    }
  }
  {
    int_cpp((i)) = (0);
    while ((((i)) < ((n))))
    {
      write(g[i], "\n");
      ((i)) += 1;
    }
  }
  return 0;
}
