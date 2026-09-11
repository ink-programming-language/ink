// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var nop: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_array(100);
  var stt: dynamic = cpp_array(3010);
  var vct: dynamic = cpp_uninitialized();
  var v2: dynamic = cpp_uninitialized();
  var v3: dynamic = cpp_uninitialized();
  read(n);
  nop = n;
  {
    i = 0;
    while ((i < n))
    {
      read(stt[i], a);
      vct.push_back(make_pair(a, stt[i]));
      v2.push_back(i);
      i += 1;
    }
  }
  v2.push_back(n);
  sort(vct.begin(), vct.end());
  {
    i = 0;
    while ((i < n))
    {
      if ((vct[i].first > i))
      {
        write(-1, "\n");
        return 0;
      }
      i += 1;
    }
  }
  var nn: dynamic = n;
  {
    i = (nn - 1);
    while ((i >= 0))
    {
      if ((vct[i].first != 0))
      {
        temp = v2[(n - vct[i].first)];
        v3.push_back(temp);
        v2.erase(((v2.begin() + n) - vct[i].first));
        n -= 1;
      }
      i -= 1;
    }
  }
  if ((v2.empty() == 0))
  {
    nn = v2.size();
    {
      i = (nn - 1);
      while ((i > 0))
      {
        v3.push_back(v2[i]);
        i -= 1;
      }
    }
  }
  {
    i = (nop - 1);
    j = 0;
    while ((i >= 0))
    {
      write(vct[((nop - i) - 1)].second, " ", v3[i], "\n");
      j += 1;
      i -= 1;
    }
  }
  return 0;
}
