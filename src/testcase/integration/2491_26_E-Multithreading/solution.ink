// Translated from solution.cpp.

var N: dynamic = 128;

func main() -> dynamic
{
  var vp: dynamic = cpp_uninitialized();
  var vl: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  scanf("%d %d", (&n), (&w));
  var CW: dynamic = w;
  var m: dynamic = cpp_array(N);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&m[i]));
      i += 1;
    }
  }
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
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
  var Q: dynamic = cpp_uninitialized();
  var vst: dynamic = [false];
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var mx: dynamic = (1 << 20);
      var mi: dynamic = -1;
      {
        var j: dynamic = 0;
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
        var j: dynamic = 0;
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
      var p: dynamic = Q.front();
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
  var dict: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < cpp_cast(vp.size())))
    {
      var cur: dynamic = vp[i];
      vl.push_back((cur + 1));
      var erl: dynamic = cpp_uninitialized();
      {
        var j: dynamic = dict.begin();
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
        var j: dynamic = 0;
        while ((j < cpp_cast(erl.size())))
        {
          {
            var k: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < cpp_cast(vl.size())))
    {
      printf("%d%c", vl[i],  ((i == (vl.size() - 1))) ? cpp_char("\n") : cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
