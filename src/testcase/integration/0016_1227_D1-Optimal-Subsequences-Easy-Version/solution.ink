// Translated from solution.cpp.

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  var v2: dynamic = v;
  sort(v.rbegin(), v.rend());
  var m: dynamic = cpp_uninitialized();
  read(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var k: dynamic = cpp_uninitialized();
      var ind: dynamic = cpp_uninitialized();
      read(k, ind);
      var kmax: dynamic = cpp_uninitialized();
      {
        var i: dynamic = 0;
        while ((i < k))
        {
          kmax.insert(v[i]);
          i += 1;
        }
      }
      var seq: dynamic = cpp_uninitialized();
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          var it: dynamic = kmax.find(v2[i]);
          if ((it != kmax.end()))
          {
            kmax.erase(it);
            seq.push_back(v2[i]);
          }
          if ((seq.size() == k))
          {
            break;
          }
          i += 1;
        }
      }
      write(seq[(ind - 1)], "\n");
      i += 1;
    }
  }
  return 0;
}
