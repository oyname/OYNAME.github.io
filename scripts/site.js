console.log('site.js geladen');

function initSlider() {
    const slides = document.querySelectorAll('.slide');
    const indicators = document.querySelectorAll('.indicator');

    if (!slides.length || !indicators.length) return;

    let currentSlide = 0;
    let slideInterval;
    const slideCount = slides.length;

    function showSlide(index) {
        slides[currentSlide].classList.remove('active');
        indicators[currentSlide].classList.remove('active');

        currentSlide = index;

        slides[currentSlide].classList.add('active');
        indicators[currentSlide].classList.add('active');
    }

    function nextSlide() {
        showSlide((currentSlide + 1) % slideCount);
    }

    indicators.forEach((indicator, index) => {
        indicator.addEventListener('click', () => {
            showSlide(index);
            resetInterval();
        });
    });

    function resetInterval() {
        clearInterval(slideInterval);
        slideInterval = setInterval(nextSlide, 5000);
    }

    resetInterval();
}

function loadMarkdown(targetId, markdownFile) {
    console.log('loadMarkdown gestartet:', targetId, markdownFile);

    const target = document.getElementById(targetId);

    if (!target) {
        console.error('Target nicht gefunden:', targetId);
        return;
    }

    fetch(markdownFile)
        .then(response => {
            console.log('fetch response:', response.status, markdownFile);
            if (!response.ok) throw new Error('Datei nicht gefunden: ' + markdownFile);
            return response.text();
        })
        .then(markdown => {
            console.log('markdown geladen');

            if (typeof showdown === 'undefined') {
                throw new Error('showdown wurde nicht geladen');
            }

            const converter = new showdown.Converter({
                tables: true,
                ghCompatibleHeaderId: true,
                simplifiedAutoLink: true,
                strikethrough: true
            });

            target.innerHTML = converter.makeHtml(markdown);
        })
        .catch(error => {
            console.error('loadMarkdown Fehler:', error);
            target.innerHTML =
                '<div class="error">❌ Fehler beim Laden: ' + error.message + '</div>';
        });
}
