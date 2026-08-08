// Translated from solution.cpp.

var N = 128;

func main()
{
  var vp: dynamic;
  var vl: dynamic;
  var n: dynamic;
  var w: dynamic;
  scanf("%d %d", (&n), (&w));
  var CW = w;
  var m = cpp_array(N);
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&m[i]));
      i += 1;
    }
  }
  var sum = 0;
  {
    var i = 0;
    while ((i < n))
    {
      sum += m[i];
      i += 1;
    }
  }
  if (((w <= 0) || (w > sum)))
  {
    printf("No\n");
    return 0;
  }
  var Q: dynamic;
  var vst = [false];
  {
    var i = 0;
    while ((i < n))
    {
      var mx = (1 << 20);
      var mi = -1;
      {
        var j = 0;
        while ((j < n))
        {
          if ((!vst[j]))
          {
            mx = min(mx, m[j]);
          }
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < n))
        {
          if (((!vst[j]) && (mx == m[j])))
          {
            mi = j;
            break;
          }
          j += 1;
        }
      }
      vst[mi] = true;
      Q.push(mi);
      i += 1;
    }
  }
  {
    while ((w != 0))
    {
      var p = Q.front();
      Q.pop();
      m[p] -= 1;
      vp.push_back(p);
      if ((m[p] != 0))
      {
        Q.push(p);
      }
      w -= 1;
    }
  }
  var dict: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      if ((m[i] != 0))
      {
        dict.insert(i);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cpp_cast(vp.size())))
    {
      var cur = vp[i];
      vl.push_back((cur + 1));
      var erl: dynamic;
      {
        var j = dict.begin();
        while ((j != dict.end()))
        {
          if (((*j) == cur))
          {
            j += 1;
            continue;
          }
          erl.push_back((*j));
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < cpp_cast(erl.size())))
        {
          {
            var k = 0;
            while ((k < (2 * m[erl[j]])))
            {
              vl.push_back((erl[j] + 1));
              k += 1;
            }
          }
          dict.erase(erl[j]);
          j += 1;
        }
      }
      vl.push_back((cur + 1));
      i += 1;
    }
  }
  if ((vl.size() != (2 * sum)))
  {
    printf("No\n");
    return 0;
  }
  printf("Yes\n");
  {
    var i = 0;
    while ((i < cpp_cast(vl.size())))
    {
      printf("%d%c", vl[i], if ((i == (vl.size() - 1))) cpp_char("\n") else cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
