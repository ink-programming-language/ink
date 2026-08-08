// Translated from solution.cpp.

func f(nums: dynamic)
{
  var k = 0;
  for (var i in nums)
  {
    k ^= i;
  }
  return k;
}

func doo(l: dynamic, r: dynamic, k: dynamic)
{
  if ((((r - l) + 1) >= 5))
  {
    if ((k >= 4))
    {
      if ((l % 2))
      {
        l += 1;
      }
      write("0\n4\n");
      {
        int_cpp(i) = 0;
        while (((i) < (4)))
        {
          write((l + i), " ");
          (i) += 1;
        }
      }
    } else if ((k == 3))
    {
      var d = -1;
      {
        var lk = r;
        while (lk)
        {
          d += 1;
          lk /= 2;
        }
      }
      var top = (1 << d);
      {
        while (top)
        {
          var x = (top | (top / 2));
          var y = (x - 1);
          var z = (x ^ y);
          if (((((((l <= x) && (l <= y)) && (l <= z)) && (r >= x)) && (r >= y)) && (r >= z)))
          {
            write("0\n3\n", x, " ", y, " ", z);
            return;
          }
          top /= 2;
        }
      }
      if ((l % 2))
      {
        l += 1;
      }
      write("1\n2\n");
      {
        int_cpp(i) = 0;
        while (((i) < (2)))
        {
          write((l + i), " ");
          (i) += 1;
        }
      }
    } else if ((k == 2))
    {
      if ((l % 2))
      {
        l += 1;
      }
      write("1\n2\n");
      {
        int_cpp(i) = 0;
        while (((i) < (2)))
        {
          write((l + i), " ");
          (i) += 1;
        }
      }
    } else
    {
      write(l, "\n1\n", l);
    }
  } else
  {
    var combs: dynamic;
    {
      int_cpp(i) = 0;
      while (((i) < ((1 << (((r - l) + 1))))))
      {
        if ((!i))
        {
          (i) += 1;
          continue;
        }
        var s: dynamic;
        {
          int_cpp(j) = 0;
          while (((j) < (((r - l) + 1))))
          {
            if ((i & ((1 << j))))
            {
              s.push_back((l + j));
            }
            (j) += 1;
          }
        }
        if ((s.size() <= k))
        {
          combs.push_back(s);
        }
        (i) += 1;
      }
    }
    var it = min_element((combs).begin(), (combs).end(), __cpp_lambda_1);
    write(f((*it)), "\n");
    write(it->size(), "\n");
    {
      int_cpp(i) = 0;
      while (((i) < (it->size())))
      {
        write(((*it))[i], " ");
        (i) += 1;
      }
    }
  }
}

func solve()
{
  var l: dynamic;
  var r: dynamic;
  var k: dynamic;
  read(l, r, k);
  doo(l, r, k);
}

func main()
{
  solve();
  return 0;
}

func __cpp_lambda_1(a1: dynamic, a2: dynamic)
{
  return (f(a1) < f(a2));
}
