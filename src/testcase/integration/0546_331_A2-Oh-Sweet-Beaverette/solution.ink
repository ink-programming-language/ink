// Translated from solution.cpp.

var mp: dynamic = cpp_uninitialized();

var vc: dynamic = cpp_uninitialized();

var psum: dynamic = cpp_array(300001);

var a: dynamic = cpp_array(300001);

func main() -> dynamic
{
  var ans: dynamic = -2000000001;
  var n: dynamic = cpp_uninitialized();
  var ansi: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      if ((a[i] > 0))
      {
        psum[i] = (psum[(i - 1)] + a[i]);
      } else
      {
        psum[i] = psum[(i - 1)];
      }
      if ((mp.find(a[i]) == mp.end()))
      {
        mp.insert(make_pair(a[i], i));
      } else
      {
        var hoge: dynamic = (psum[i] - psum[(mp[a[i]] - 1)]);
        if ((a[i] < 0))
        {
          hoge += ((2 * a[i]));
        }
        if ((ans < hoge))
        {
          ans = hoge;
          ansi = i;
        }
      }
      i += 1;
    }
  }
  write(ans);
  var j: dynamic = 1;
  var cnt: dynamic = 0;
  {
    while ((j <= n))
    {
      if ((a[j] == a[ansi]))
      {
        break;
      }
      vc.push_back(j);
      j += 1;
    }
  }
  {
    j += 1;
    while ((j <= (ansi - 1)))
    {
      if ((a[j] < 0))
      {
        vc.push_back(j);
      }
      j += 1;
    }
  }
  {
    j = (ansi + 1);
    while ((j <= n))
    {
      vc.push_back(j);
      j += 1;
    }
  }
  write(" ", vc.size(), "\n");
  {
    var i: dynamic = 0;
    while ((i < vc.size()))
    {
      write(vc[i], " ");
      i += 1;
    }
  }
  return 0;
}
