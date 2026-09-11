// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var arr: dynamic = cpp_array(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(arr[i]);
        i += 1;
      }
    }
    if ((n < 6))
    {
      write(0, " ", 0, " ", 0, "\n");
      continue;
    }
    var index: dynamic = (((n / 2)) - 1);
    while (((index >= 0) && (arr[index] == arr[(index + 1)])))
    {
      index -= 1;
    }
    var count: dynamic = cpp_uninitialized();
    var sum: dynamic = 0;
    {
      var i: dynamic = index;
      while ((i >= 0))
      {
        var temp: dynamic = 1;
        var j: dynamic = (i - 1);
        while (((j >= 0) && (arr[j] == arr[i])))
        {
          j -= 1;
          temp += 1;
        }
        count.push_back(temp);
        sum += temp;
        i = (j + 1);
        i -= 1;
      }
    }
    if ((count.size() < 3))
    {
      write(0, " ", 0, " ", 0, "\n");
      continue;
    }
    sum -= count[(count.size() - 1)];
    var g: dynamic = count[(count.size() - 1)];
    var s: dynamic = 0;
    var b: dynamic = 0;
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < (count.size() - 1)))
      {
        b += count[i];
        s = (sum - b);
        if (((g < b) && (g < s)))
        {
          ans = 1;
          break;
        }
        i += 1;
      }
    }
    if ((ans == 1))
    {
      write(g, " ", s, " ", b, "\n");
    } else
    {
      write(0, " ", 0, " ", 0, "\n");
    }
  }
  return 0;
}
