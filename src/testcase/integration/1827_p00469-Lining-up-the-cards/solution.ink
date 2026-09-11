// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var num: dynamic = cpp_array(5);

var card: dynamic = cpp_array(11);

var ans: dynamic = cpp_uninitialized();

func serch(m: dynamic, k: dynamic, now: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < m))
        {
          if ((num[i] == num[j]))
          {
            return;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((m == k))
  {
    var str: dynamic = cpp_uninitialized();
    var ch: dynamic = cpp_array(8);
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        sprintf(ch, "%d", card[num[i]]);
        str += string_cpp(ch);
        i += 1;
      }
    }
    ans.insert(str);
    return;
  }
  num[m] = now;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      serch((m + 1), k, i);
      i += 1;
    }
  }
  return;
}

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    while (true)
    {
      read(n, k);
      if (((n == 0) && (k == 0)))
      {
        break;
      }
      ans.clear();
      {
        i = 0;
        while ((i < n))
        {
          read(card[i]);
          i += 1;
        }
      }
      {
        i = 0;
        while ((i < n))
        {
          serch(0, k, i);
          i += 1;
        }
      }
      var it: dynamic = ans.begin();
      write(ans.size(), "\n");
    }
  }
}
