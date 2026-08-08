// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var arr = cpp_array(n);
    {
      var i = 0;
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
    var index = (((n / 2)) - 1);
    while (((index >= 0) && (arr[index] == arr[(index + 1)])))
    {
      index -= 1;
    }
    var count: dynamic;
    var sum = 0;
    {
      var i = index;
      while ((i >= 0))
      {
        var temp = 1;
        var j = (i - 1);
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
    var g = count[(count.size() - 1)];
    var s = 0;
    var b = 0;
    var ans = 0;
    {
      var i = 0;
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
