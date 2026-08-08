// Translated from solution.cpp.

var mp: dynamic;

var vc: dynamic;

var psum = cpp_array(300001);

var a = cpp_array(300001);

func main()
{
  var ans = -2000000001;
  var n: dynamic;
  var ansi: dynamic;
  read(n);
  {
    var i = 1;
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
        var hoge = (psum[i] - psum[(mp[a[i]] - 1)]);
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
  var j = 1;
  var cnt = 0;
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
    var i = 0;
    while ((i < vc.size()))
    {
      write(vc[i], " ");
      i += 1;
    }
  }
  return 0;
}
