// Translated from solution.cpp.

var n: dynamic;

var num = cpp_array(5);

var card = cpp_array(11);

var ans: dynamic;

func serch(m: dynamic, k: dynamic, now: dynamic)
{
  {
    var i = 0;
    while ((i < m))
    {
      {
        var j = (i + 1);
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
    var str: dynamic;
    var ch = cpp_array(8);
    {
      var i = 0;
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
    var i = 0;
    while ((i < n))
    {
      serch((m + 1), k, i);
      i += 1;
    }
  }
  return;
}

func main()
{
  var k: dynamic;
  var i: dynamic;
  var j: dynamic;
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
      var it = ans.begin();
      write(ans.size(), "\n");
    }
  }
}
