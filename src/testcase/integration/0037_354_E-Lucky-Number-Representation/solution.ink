// Translated from solution.cpp.

var kharab: dynamic = [1, 2, 3, 5, 6, 9, 10, 13, 17, 31, 34, 37, 38, 41, 43, 45, 46, 49, 50, 53, 57, 71, 83, 111, 123, 391, 403, 437, 457, 471, 483, 511, 523];

var a: dynamic = cpp_array(6);

var D: dynamic = cpp_array(43);

func f(n: dynamic) -> dynamic
{
  if ((kharab.find(n) != kharab.end()))
  {
    fill(a, (a + 6), -1);
    return;
  }
  if ((n < 8))
  {
    a[0] = n;
    return;
  }
  {
    var i: dynamic = 0;
    while ((i <= 42))
    {
      if ((((((!D[i].size()) || (~D[i].back()))) && ((i % 10) == (n % 10))) && (kharab.find((((n - i)) / 10)) == kharab.end())))
      {
        f((((n - i)) / 10));
        {
          var j: dynamic = 0;
          while ((j < 6))
          {
            a[j] *= 10;
            j += 1;
          }
        }
        {
          var j: dynamic = 0;
          while ((j < D[i].size()))
          {
            a[j] += D[i][j];
            j += 1;
          }
        }
        return;
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  D[4].push_back(4);
  D[7].push_back(7);
  {
    var i: dynamic = 1;
    while ((i <= 42))
    {
      if (((i == 4) || (i == 7)))
      {
        i += 1;
        continue;
      }
      if (((kharab.find(i) != kharab.end()) || (i == 40)))
      {
        D[i].push_back(-1);
        i += 1;
        continue;
      }
      if (((D[(i - 4)].size() < 6) && (((!D[(i - 4)].size()) || (~D[(i - 4)].back())))))
      {
        D[i] = D[(i - 4)];
        D[i].push_back(4);
      } else
      {
        D[i] = D[(i - 7)];
        D[i].push_back(7);
      }
      i += 1;
    }
  }
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    fill(a, (a + 6), 0);
    f(n);
    if ((~a[0]))
    {
      {
        var i: dynamic = 0;
        while ((i < 6))
        {
          write(a[i], cpp_char(" "));
          i += 1;
        }
      }
    } else
    {
      write(-1);
    }
    write(cpp_char("\n"));
  }
}
